use clap::Parser;

#[derive(Debug, Parser)]
#[command(
    name = "netop",
    version,
    about = "Network resource monitor and control TUI (htop-style)",
    long_about = "Real-time interface bandwidth and per-process socket activity on Linux."
)]
pub struct Cli {
    /// Refresh interval in milliseconds.
    #[arg(short = 'i', long, default_value_t = 500)]
    pub interval_ms: u64,

    /// Start on the process view instead of interfaces.
    #[arg(short = 'p', long)]
    pub process_view: bool,
}
