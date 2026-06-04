use ratatui::{
    layout::{Constraint, Direction, Layout, Rect},
    style::{Color, Modifier, Style},
    text::{Line, Span},
    widgets::{Block, Borders, Cell, Paragraph, Row, Table, Wrap},
    Frame,
};

use super::model::{App, ConfirmKind, ViewMode};
use crate::util::format_rate;

pub fn draw(frame: &mut Frame, app: &App) {
    let area = frame.area();
    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([
            Constraint::Length(3),
            Constraint::Min(5),
            Constraint::Length(3),
        ])
        .split(area);

    draw_header(frame, app, chunks[0]);
    match app.view {
        ViewMode::Interface => draw_interface_table(frame, app, chunks[1]),
        ViewMode::Process => draw_process_table(frame, app, chunks[1]),
    }
    draw_footer(frame, app, chunks[2]);

    if let Some(kind) = &app.confirm {
        draw_confirm(frame, kind);
    }
}

fn draw_header(frame: &mut Frame, app: &App, area: Rect) {
    let view_label = match app.view {
        ViewMode::Interface => "Interfaces",
        ViewMode::Process => "Processes",
    };
    let pause = if app.paused { " [PAUSED]" } else { "" };
    let filter = if app.filter.is_empty() {
        String::new()
    } else {
        format!(" filter={}", app.filter)
    };

    let rx = format_rate(app.snapshot.total_rx_bps());
    let tx = format_rate(app.snapshot.total_tx_bps());

    let line = Line::from(vec![
        Span::styled(" netop ", Style::default().fg(Color::Cyan).add_modifier(Modifier::BOLD)),
        Span::raw(format!("| {view_label}{pause} | sort: {} | ↓RX {rx} ↑TX {tx}{filter}", app.sort.label())),
    ]);

    let block = Block::default()
        .borders(Borders::ALL)
        .title(" Network monitor ");
    frame.render_widget(Paragraph::new(line).block(block), area);
}

fn draw_interface_table(frame: &mut Frame, app: &App, area: Rect) {
    let visible_rows = area.height.saturating_sub(3) as usize;
    let rows_data = app.filtered_interfaces();
    let header = Row::new(vec![
        Cell::from("Interface"),
        Cell::from("↓ RX/s"),
        Cell::from("↑ TX/s"),
        Cell::from("↓ PPS"),
        Cell::from("↑ PPS"),
        Cell::from("Err"),
        Cell::from("Drop"),
    ])
    .style(Style::default().add_modifier(Modifier::BOLD));

    let start = app.scroll_offset.min(rows_data.len().saturating_sub(1));
    let end = (start + visible_rows).min(rows_data.len());

    let rows: Vec<Row> = rows_data[start..end]
        .iter()
        .enumerate()
        .map(|(i, s)| {
            let selected = start + i == app.selected;
            let style = if selected {
                Style::default().bg(Color::DarkGray)
            } else {
                Style::default()
            };
            Row::new(vec![
                Cell::from(s.name.clone()),
                Cell::from(format_rate(s.rx_bps)),
                Cell::from(format_rate(s.tx_bps)),
                Cell::from(format!("{:.0}", s.rx_pps)),
                Cell::from(format!("{:.0}", s.tx_pps)),
                Cell::from(format!("{}/{}", s.rx_errors, s.tx_errors)),
                Cell::from(format!("{}/{}", s.rx_dropped, s.tx_dropped)),
            ])
            .style(style)
        })
        .collect();

    let table = Table::new(rows, [
        Constraint::Length(14),
        Constraint::Length(10),
        Constraint::Length(10),
        Constraint::Length(8),
        Constraint::Length(8),
        Constraint::Length(8),
        Constraint::Length(8),
    ])
    .header(header)
    .block(
        Block::default()
            .borders(Borders::ALL)
            .title(format!(" Interfaces ({}) ", rows_data.len())),
    );

    frame.render_widget(table, area);
}

fn draw_process_table(frame: &mut Frame, app: &App, area: Rect) {
    let visible_rows = area.height.saturating_sub(3) as usize;
    let rows_data = app.filtered_processes();
    let header = Row::new(vec![
        Cell::from("PID"),
        Cell::from("Name"),
        Cell::from("Activity/s"),
        Cell::from("TCP"),
        Cell::from("UDP"),
        Cell::from("Queue B"),
    ])
    .style(Style::default().add_modifier(Modifier::BOLD));

    let start = app.scroll_offset.min(rows_data.len().saturating_sub(1));
    let end = (start + visible_rows).min(rows_data.len());

    let rows: Vec<Row> = rows_data[start..end]
        .iter()
        .enumerate()
        .map(|(i, p)| {
            let selected = start + i == app.selected;
            let style = if selected {
                Style::default().bg(Color::DarkGray)
            } else {
                Style::default()
            };
            Row::new(vec![
                Cell::from(p.pid.to_string()),
                Cell::from(truncate(&p.name, 24)),
                Cell::from(format_rate(p.activity_bps)),
                Cell::from(p.tcp_conns.to_string()),
                Cell::from(p.udp_socks.to_string()),
                Cell::from(format_bytes(p.queue_bytes)),
            ])
            .style(style)
        })
        .collect();

    let table = Table::new(rows, [
        Constraint::Length(8),
        Constraint::Length(26),
        Constraint::Length(12),
        Constraint::Length(6),
        Constraint::Length(6),
        Constraint::Length(10),
    ])
    .header(header)
    .block(
        Block::default()
            .borders(Borders::ALL)
            .title(format!(" Processes ({}) ", rows_data.len())),
    );

    frame.render_widget(table, area);
}

fn draw_footer(frame: &mut Frame, app: &App, area: Rect) {
    let filter_hint = if app.filter_input {
        format!("> {}", app.filter)
    } else {
        app.status.clone()
    };
    let block = Block::default().borders(Borders::ALL);
    frame.render_widget(
        Paragraph::new(filter_hint)
            .block(block)
            .wrap(Wrap { trim: true }),
        area,
    );
}

fn draw_confirm(frame: &mut Frame, kind: &ConfirmKind) {
    let area = centered_rect(50, 20, frame.area());
    let text = match kind {
            ConfirmKind::Kill { pid } => format!("Send SIGTERM to PID {pid}?  [y/N]"),
        };
    frame.render_widget(
        Paragraph::new(text)
            .block(
                Block::default()
                    .borders(Borders::ALL)
                    .title(" Confirm ")
                    .style(Style::default().fg(Color::Yellow)),
            )
            .wrap(Wrap { trim: true }),
        area,
    );
}

fn centered_rect(percent_x: u16, percent_y: u16, area: Rect) -> Rect {
    let popup_layout = Layout::default()
        .direction(Direction::Vertical)
        .constraints([
            Constraint::Percentage((100 - percent_y) / 2),
            Constraint::Percentage(percent_y),
            Constraint::Percentage((100 - percent_y) / 2),
        ])
        .split(area);

    Layout::default()
        .direction(Direction::Horizontal)
        .constraints([
            Constraint::Percentage((100 - percent_x) / 2),
            Constraint::Percentage(percent_x),
            Constraint::Percentage((100 - percent_x) / 2),
        ])
        .split(popup_layout[1])[1]
}

fn truncate(s: &str, max: usize) -> String {
    if s.chars().count() <= max {
        s.to_string()
    } else {
        format!("{}…", s.chars().take(max.saturating_sub(1)).collect::<String>())
    }
}

fn format_bytes(n: u64) -> String {
    const K: f64 = 1024.0;
    let b = n as f64;
    if b >= K * K {
        format!("{:.1}M", b / (K * K))
    } else if b >= K {
        format!("{:.1}K", b / K)
    } else {
        format!("{n}")
    }
}
