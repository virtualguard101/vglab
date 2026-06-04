use crossterm::event::{KeyCode, KeyEvent, KeyModifiers};

use crate::collector::snapshot::Snapshot;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ViewMode {
    Interface,
    Process,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SortKey {
    Bandwidth,
    Pps,
    Name,
    Errors,
    Connections,
    Pid,
}

impl SortKey {
    pub fn label(self) -> &'static str {
        match self {
            Self::Bandwidth => "bandwidth",
            Self::Pps => "pps",
            Self::Name => "name",
            Self::Errors => "errors",
            Self::Connections => "connections",
            Self::Pid => "pid",
        }
    }

    pub fn next(self, view: ViewMode) -> Self {
        match view {
            ViewMode::Interface => match self {
                Self::Bandwidth => Self::Pps,
                Self::Pps => Self::Name,
                Self::Name => Self::Errors,
                Self::Errors => Self::Bandwidth,
                _ => Self::Bandwidth,
            },
            ViewMode::Process => match self {
                Self::Bandwidth => Self::Connections,
                Self::Connections => Self::Pid,
                Self::Pid => Self::Name,
                Self::Name => Self::Bandwidth,
                _ => Self::Bandwidth,
            },
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ConfirmKind {
    Kill { pid: u32 },
}

#[derive(Debug, Clone)]
pub enum Message {
    Quit,
    ConfirmYes,
    ConfirmNo,
}

#[derive(Debug, Clone)]
pub struct App {
    pub view: ViewMode,
    pub sort: SortKey,
    pub filter: String,
    pub filter_input: bool,
    pub selected: usize,
    pub snapshot: Snapshot,
    pub status: String,
    pub confirm: Option<ConfirmKind>,
    pub paused: bool,
    pub scroll_offset: usize,
}

impl App {
    pub fn new(process_view: bool) -> Self {
        Self {
            view: if process_view {
                ViewMode::Process
            } else {
                ViewMode::Interface
            },
            sort: SortKey::Bandwidth,
            filter: String::new(),
            filter_input: false,
            selected: 0,
            snapshot: Snapshot::default(),
            status: String::from("netop — press ? for help"),
            confirm: None,
            paused: false,
            scroll_offset: 0,
        }
    }

    pub fn filtered_interfaces(&self) -> Vec<&crate::collector::snapshot::InterfaceStat> {
        let q = self.filter.to_lowercase();
        let mut rows: Vec<_> = self
            .snapshot
            .interfaces
            .iter()
            .filter(|i| q.is_empty() || i.name.to_lowercase().contains(&q))
            .collect();
        sort_interfaces(&mut rows, self.sort);
        rows
    }

    pub fn filtered_processes(&self) -> Vec<&crate::collector::snapshot::ProcessStat> {
        let q = self.filter.to_lowercase();
        let mut rows: Vec<_> = self
            .snapshot
            .processes
            .iter()
            .filter(|p| {
                q.is_empty()
                    || p.name.to_lowercase().contains(&q)
                    || p.pid.to_string().contains(&q)
            })
            .collect();
        sort_processes(&mut rows, self.sort);
        rows
    }

    pub fn clamp_selection(&mut self) {
        let len = match self.view {
            ViewMode::Interface => self.filtered_interfaces().len(),
            ViewMode::Process => self.filtered_processes().len(),
        };
        if len == 0 {
            self.selected = 0;
            self.scroll_offset = 0;
            return;
        }
        if self.selected >= len {
            self.selected = len - 1;
        }
        if self.scroll_offset > self.selected {
            self.scroll_offset = self.selected;
        }
    }
}

fn sort_interfaces(
    rows: &mut Vec<&crate::collector::snapshot::InterfaceStat>,
    key: SortKey,
) {
    rows.sort_by(|a, b| match key {
        SortKey::Bandwidth => b
            .rx_bps
            .max(b.tx_bps)
            .total_cmp(&a.rx_bps.max(a.tx_bps)),
        SortKey::Pps => b
            .rx_pps
            .max(b.tx_pps)
            .total_cmp(&a.rx_pps.max(a.tx_pps)),
        SortKey::Name => a.name.cmp(&b.name),
        SortKey::Errors => (b.rx_errors + b.tx_errors).cmp(&(a.rx_errors + a.tx_errors)),
        _ => b.rx_bps.total_cmp(&a.rx_bps),
    });
}

fn sort_processes(
    rows: &mut Vec<&crate::collector::snapshot::ProcessStat>,
    key: SortKey,
) {
    rows.sort_by(|a, b| match key {
        SortKey::Bandwidth => b.activity_bps.total_cmp(&a.activity_bps),
        SortKey::Connections => (b.tcp_conns + b.udp_socks).cmp(&(a.tcp_conns + a.udp_socks)),
        SortKey::Pid => a.pid.cmp(&b.pid),
        SortKey::Name => a.name.cmp(&b.name),
        _ => b.activity_bps.total_cmp(&a.activity_bps),
    });
}

pub fn handle_key(app: &mut App, key: KeyEvent) -> Option<Message> {
    if app.filter_input {
        return handle_filter_key(app, key);
    }

    if app.confirm.is_some() {
        return handle_confirm_key(app, key);
    }

    match key.code {
        KeyCode::Char('q') | KeyCode::Esc => Some(Message::Quit),
        KeyCode::Char('?') => {
            app.status = help_text().into();
            None
        }
        KeyCode::Char('1') => {
            app.view = ViewMode::Interface;
            app.sort = SortKey::Bandwidth;
            app.clamp_selection();
            None
        }
        KeyCode::Char('2') => {
            app.view = ViewMode::Process;
            app.sort = SortKey::Bandwidth;
            app.clamp_selection();
            None
        }
        KeyCode::Tab => {
            app.view = match app.view {
                ViewMode::Interface => ViewMode::Process,
                ViewMode::Process => ViewMode::Interface,
            };
            app.clamp_selection();
            None
        }
        KeyCode::Char('s') => {
            app.sort = app.sort.next(app.view);
            app.status = format!("sort: {}", app.sort.label());
            app.clamp_selection();
            None
        }
        KeyCode::Char('/') => {
            app.filter_input = true;
            app.status = "filter: type pattern, Enter to apply, Esc to cancel".into();
            None
        }
        KeyCode::Char('c') if key.modifiers.contains(KeyModifiers::CONTROL) => Some(Message::Quit),
        KeyCode::Char(' ') => {
            app.paused = !app.paused;
            app.status = if app.paused {
                "paused".into()
            } else {
                "resumed".into()
            };
            None
        }
        KeyCode::Char('k') => {
            app.selected = app.selected.saturating_sub(1);
            if app.selected < app.scroll_offset {
                app.scroll_offset = app.selected;
            }
            None
        }
        KeyCode::Char('j') => {
            let len = match app.view {
                ViewMode::Interface => app.filtered_interfaces().len(),
                ViewMode::Process => app.filtered_processes().len(),
            };
            if len > 0 {
                app.selected = (app.selected + 1).min(len - 1);
            }
            None
        }
        KeyCode::F(9) => {
            if app.view == ViewMode::Process {
                let target = app
                    .filtered_processes()
                    .get(app.selected)
                    .map(|p| (p.pid, p.name.clone()));
                if let Some((pid, name)) = target {
                    if pid == crate::control::kill::own_pid() {
                        app.status = "refusing to kill netop itself".into();
                    } else {
                        app.confirm = Some(ConfirmKind::Kill { pid });
                        app.status = format!("kill pid {pid} ({name})? [y/N]");
                    }
                }
            }
            None
        }
        KeyCode::Up => {
            app.selected = app.selected.saturating_sub(1);
            if app.selected < app.scroll_offset {
                app.scroll_offset = app.selected;
            }
            None
        }
        KeyCode::Down => {
            let len = match app.view {
                ViewMode::Interface => app.filtered_interfaces().len(),
                ViewMode::Process => app.filtered_processes().len(),
            };
            if len > 0 {
                app.selected = (app.selected + 1).min(len - 1);
            }
            None
        }
        KeyCode::PageUp => {
            app.selected = app.selected.saturating_sub(10);
            app.scroll_offset = app.scroll_offset.saturating_sub(10);
            None
        }
        KeyCode::PageDown => {
            let len = match app.view {
                ViewMode::Interface => app.filtered_interfaces().len(),
                ViewMode::Process => app.filtered_processes().len(),
            };
            if len > 0 {
                app.selected = (app.selected + 10).min(len - 1);
                app.scroll_offset = (app.scroll_offset + 10).min(app.selected);
            }
            None
        }
        _ => None,
    }
}

fn handle_filter_key(app: &mut App, key: KeyEvent) -> Option<Message> {
    match key.code {
        KeyCode::Enter => {
            app.filter_input = false;
            app.selected = 0;
            app.scroll_offset = 0;
            app.status = if app.filter.is_empty() {
                "filter cleared".into()
            } else {
                format!("filter: {}", app.filter)
            };
            None
        }
        KeyCode::Esc => {
            app.filter_input = false;
            app.filter.clear();
            app.status = "filter cancelled".into();
            None
        }
        KeyCode::Backspace => {
            app.filter.pop();
            None
        }
        KeyCode::Char(c) => {
            app.filter.push(c);
            None
        }
        _ => None,
    }
}

fn handle_confirm_key(_app: &mut App, key: KeyEvent) -> Option<Message> {
    match key.code {
        KeyCode::Char('y') | KeyCode::Char('Y') => Some(Message::ConfirmYes),
        KeyCode::Char('n') | KeyCode::Char('N') | KeyCode::Esc => Some(Message::ConfirmNo),
        _ => None,
    }
}

fn help_text() -> &'static str {
    "[1] iface [2] proc Tab switch | j/k ↑↓ move | s sort | / filter | F9 kill | Space pause | q quit"
}
