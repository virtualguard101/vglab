mod model;
mod view;

use std::io::stdout;
use std::time::Duration;

use anyhow::Result;
use crossterm::event::{self, Event, KeyEventKind};
use crossterm::terminal::{
    disable_raw_mode, enable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen,
};
use crossterm::ExecutableCommand;
use ratatui::backend::CrosstermBackend;
use ratatui::Terminal;

use crate::cli::Cli;
use crate::collector::Collector;
use crate::control::{self, ControlAction};

pub use model::App;

pub fn run(cli: Cli) -> Result<()> {
    enable_raw_mode()?;
    stdout().execute(EnterAlternateScreen)?;
    let mut terminal = Terminal::new(CrosstermBackend::new(stdout()))?;

    let mut app = App::new(cli.process_view);
    let mut collector = Collector::new();
    let interval = Duration::from_millis(cli.interval_ms);
    let mut last_poll = std::time::Instant::now();

    // Prime counters so the first frame has baseline deltas.
    let _ = collector.poll();

    loop {
        let elapsed = last_poll.elapsed();
        if elapsed >= interval && !app.paused {
            if let Ok(snapshot) = collector.poll() {
                app.snapshot = snapshot;
                app.clamp_selection();
            }
            last_poll = std::time::Instant::now();
        }

        terminal.draw(|f| view::draw(f, &app))?;

        if event::poll(Duration::from_millis(50))? {
            match event::read()? {
                Event::Key(key) if key.kind == KeyEventKind::Press => {
                    if let Some(msg) = model::handle_key(&mut app, key) {
                        match msg {
                            model::Message::Quit => break,
                            model::Message::ConfirmYes => {
                                if let Some(model::ConfirmKind::Kill { pid }) = app.confirm.take()
                                {
                                    match control::execute(ControlAction::KillProcess {
                                        pid,
                                        signal: libc::SIGTERM,
                                    }) {
                                        Ok(()) => app.status = format!("sent SIGTERM to {pid}"),
                                        Err(e) => app.status = format!("kill failed: {e:#}"),
                                    }
                                }
                            }
                            model::Message::ConfirmNo => {
                                app.confirm = None;
                                app.status = "cancelled".into();
                            }
                        }
                    }
                }
                Event::Resize(_, _) => {}
                _ => {}
            }
        }
    }

    disable_raw_mode()?;
    stdout().execute(LeaveAlternateScreen)?;
    terminal.show_cursor()?;
    Ok(())
}
