use std::collections::HashMap;
use std::fs;
use std::path::{Path, PathBuf};
use std::time::Duration;

use anyhow::{Context, Result};

use super::snapshot::ProcessStat;

const PROC_NET_TCP: &str = "/proc/net/tcp";
const PROC_NET_TCP6: &str = "/proc/net/tcp6";
const PROC_NET_UDP: &str = "/proc/net/udp";
const PROC_NET_UDP6: &str = "/proc/net/udp6";

#[derive(Debug, Default, Clone, Copy)]
struct SocketAgg {
    tcp: u32,
    udp: u32,
    queue_bytes: u64,
}

/// Maps socket inodes to owning PIDs and aggregates per-process socket stats.
pub struct ProcessCollector {
    prev_queue: HashMap<u32, u64>,
}

impl ProcessCollector {
    pub fn new() -> Self {
        Self {
            prev_queue: HashMap::new(),
        }
    }

    pub fn poll(&mut self, elapsed: Duration) -> Result<Vec<ProcessStat>> {
        let inode_to_pid = build_inode_pid_map()?;
        let mut per_pid: HashMap<u32, SocketAgg> = HashMap::new();

        for path in [PROC_NET_TCP, PROC_NET_TCP6] {
            merge_socket_file(path, false, &inode_to_pid, &mut per_pid)?;
        }
        for path in [PROC_NET_UDP, PROC_NET_UDP6] {
            merge_socket_file(path, true, &inode_to_pid, &mut per_pid)?;
        }

        let names = read_process_names(per_pid.keys().copied().collect())?;
        let secs = elapsed.as_secs_f64().max(1e-6);
        let mut stats = Vec::with_capacity(per_pid.len());

        for (pid, agg) in per_pid {
            let prev_q = self.prev_queue.get(&pid).copied().unwrap_or(0);
            let activity_bps = agg.queue_bytes.saturating_sub(prev_q) as f64 / secs;
            stats.push(ProcessStat {
                pid,
                name: names.get(&pid).cloned().unwrap_or_else(|| "?".into()),
                tcp_conns: agg.tcp,
                udp_socks: agg.udp,
                queue_bytes: agg.queue_bytes,
                activity_bps,
            });
        }

        self.prev_queue = stats
            .iter()
            .map(|s| (s.pid, s.queue_bytes))
            .collect();

        stats.sort_by(|a, b| b.activity_bps.total_cmp(&a.activity_bps));
        Ok(stats)
    }
}

fn merge_socket_file(
    path: &str,
    is_udp: bool,
    inode_to_pid: &HashMap<u64, u32>,
    per_pid: &mut HashMap<u32, SocketAgg>,
) -> Result<()> {
    let text = match fs::read_to_string(path) {
        Ok(t) => t,
        Err(e) if e.kind() == std::io::ErrorKind::NotFound => return Ok(()),
        Err(e) => return Err(e).with_context(|| format!("read {path}")),
    };

    for line in text.lines().skip(1) {
        let cols: Vec<&str> = line.split_whitespace().collect();
        if cols.len() < 10 {
            continue;
        }
        let tx_queue = parse_hex(cols[4]);
        let rx_queue = parse_hex(cols[5]);
        let inode: u64 = cols[9].parse().unwrap_or(0);
        if inode == 0 {
            continue;
        }
        let Some(&pid) = inode_to_pid.get(&inode) else {
            continue;
        };

        let entry = per_pid.entry(pid).or_default();
        if is_udp {
            entry.udp += 1;
        } else {
            entry.tcp += 1;
        }
        entry.queue_bytes += tx_queue + rx_queue;
    }
    Ok(())
}

fn parse_hex(s: &str) -> u64 {
    u64::from_str_radix(s, 16).unwrap_or(0)
}

fn build_inode_pid_map() -> Result<HashMap<u64, u32>> {
    let mut map = HashMap::new();
    let proc = Path::new("/proc");

    let entries = fs::read_dir(proc).context("read /proc")?;
    for entry in entries.flatten() {
        let file_name = entry.file_name();
        let Some(pid_str) = file_name.to_str() else {
            continue;
        };
        let Ok(pid) = pid_str.parse::<u32>() else {
            continue;
        };

        let fd_dir = entry.path().join("fd");
        let Ok(fds) = fs::read_dir(&fd_dir) else {
            continue;
        };

        for fd in fds.flatten() {
            let Ok(link) = fs::read_link(fd.path()) else {
                continue;
            };
            if let Some(inode) = socket_inode_from_link(&link) {
                map.insert(inode, pid);
            }
        }
    }
    Ok(map)
}

fn socket_inode_from_link(path: &Path) -> Option<u64> {
    let s = path.to_str()?;
    let rest = s.strip_prefix("socket:[")?;
    let num = rest.strip_suffix(']')?;
    num.parse().ok()
}

fn read_process_names(pids: Vec<u32>) -> Result<HashMap<u32, String>> {
    let mut names = HashMap::new();
    for pid in pids {
        let comm_path = PathBuf::from(format!("/proc/{pid}/comm"));
        match fs::read_to_string(&comm_path) {
            Ok(s) => {
                names.insert(pid, s.trim().to_string());
            }
            Err(_) => {
                names.insert(pid, format!("pid-{pid}"));
            }
        }
    }
    Ok(names)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parse_hex_queue() {
        assert_eq!(parse_hex("1A"), 26);
        assert_eq!(parse_hex("0"), 0);
    }

    #[test]
    fn socket_inode_from_link_parses() {
        assert_eq!(
            socket_inode_from_link(Path::new("socket:[12345]")),
            Some(12345)
        );
        assert_eq!(socket_inode_from_link(Path::new("/foo")), None);
    }
}
