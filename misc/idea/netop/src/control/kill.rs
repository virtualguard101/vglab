use std::process;

use anyhow::Result;

pub fn send_signal(pid: u32, signal: i32) -> Result<()> {
    let ret = unsafe { libc::kill(pid as i32, signal) };
    if ret != 0 {
        anyhow::bail!(
            "failed to send signal {signal} to pid {pid}: {}",
            std::io::Error::last_os_error()
        );
    }
    Ok(())
}

pub fn own_pid() -> u32 {
    process::id()
}
