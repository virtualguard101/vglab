pub mod interface;
pub mod process;
pub mod snapshot;

use std::time::Instant;

use anyhow::Result;

use self::interface::InterfaceCollector;
use self::process::ProcessCollector;
use self::snapshot::Snapshot;

/// Aggregates interface and process collectors into periodic snapshots.
pub struct Collector {
    interfaces: InterfaceCollector,
    processes: ProcessCollector,
    last_tick: Instant,
}

impl Collector {
    pub fn new() -> Self {
        Self {
            interfaces: InterfaceCollector::new(),
            processes: ProcessCollector::new(),
            last_tick: Instant::now(),
        }
    }

    pub fn poll(&mut self) -> Result<Snapshot> {
        let now = Instant::now();
        let elapsed = now.duration_since(self.last_tick);
        self.last_tick = now;

        let interfaces = self.interfaces.poll(elapsed)?;
        let processes = self.processes.poll(elapsed)?;

        Ok(Snapshot {
            interfaces,
            processes,
            elapsed,
        })
    }
}

impl Default for Collector {
    fn default() -> Self {
        Self::new()
    }
}
