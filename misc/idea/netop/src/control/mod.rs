pub mod kill;

use anyhow::Result;

/// Actions that mutate system state (require appropriate privileges).
pub enum ControlAction {
    KillProcess { pid: u32, signal: i32 },
}

pub fn execute(action: ControlAction) -> Result<()> {
    match action {
        ControlAction::KillProcess { pid, signal } => kill::send_signal(pid, signal),
    }
}