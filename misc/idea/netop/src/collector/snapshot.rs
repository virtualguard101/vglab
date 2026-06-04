//! Snapshot types shared between collectors and the UI model.

use std::time::Duration;

/// One network interface with counters and derived rates.
#[derive(Debug, Clone)]
pub struct InterfaceStat {
    pub name: String,
    pub rx_bytes: u64,
    pub tx_bytes: u64,
    pub rx_packets: u64,
    pub tx_packets: u64,
    pub rx_errors: u64,
    pub tx_errors: u64,
    pub rx_dropped: u64,
    pub tx_dropped: u64,
    pub rx_bps: f64,
    pub tx_bps: f64,
    pub rx_pps: f64,
    pub tx_pps: f64,
}

/// Per-process network activity (socket-based; MVP without eBPF).
#[derive(Debug, Clone)]
pub struct ProcessStat {
    pub pid: u32,
    pub name: String,
    pub tcp_conns: u32,
    pub udp_socks: u32,
    /// Sum of socket queue bytes (tx+rx); delta used as activity proxy.
    pub queue_bytes: u64,
    pub activity_bps: f64,
}

/// Full sample taken on each refresh tick.
#[derive(Debug, Clone, Default)]
pub struct Snapshot {
    pub interfaces: Vec<InterfaceStat>,
    pub processes: Vec<ProcessStat>,
    #[allow(dead_code)]
    pub elapsed: Duration,
}

impl Snapshot {
    pub fn total_rx_bps(&self) -> f64 {
        self.interfaces.iter().map(|i| i.rx_bps).sum()
    }

    pub fn total_tx_bps(&self) -> f64 {
        self.interfaces.iter().map(|i| i.tx_bps).sum()
    }
}
