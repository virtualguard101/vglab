use std::collections::HashMap;
use std::fs;
use std::time::Duration;

use anyhow::{Context, Result};

use super::snapshot::InterfaceStat;

const PROC_NET_DEV: &str = "/proc/net/dev";

/// Reads `/proc/net/dev` and computes per-interface rates from counter deltas.
pub struct InterfaceCollector {
    prev: HashMap<String, RawCounters>,
}

#[derive(Debug, Clone, Copy)]
struct RawCounters {
    rx_bytes: u64,
    tx_bytes: u64,
    rx_packets: u64,
    tx_packets: u64,
    rx_errors: u64,
    tx_errors: u64,
    rx_dropped: u64,
    tx_dropped: u64,
}

impl InterfaceCollector {
    pub fn new() -> Self {
        Self {
            prev: HashMap::new(),
        }
    }

    pub fn poll(&mut self, elapsed: Duration) -> Result<Vec<InterfaceStat>> {
        let current = read_proc_net_dev()?;
        let secs = elapsed.as_secs_f64().max(1e-6);

        let mut stats = Vec::with_capacity(current.len());
        for (name, raw) in current {
            let (rx_bps, tx_bps, rx_pps, tx_pps) = if let Some(prev) = self.prev.get(&name) {
                (
                    rate(raw.rx_bytes, prev.rx_bytes, secs),
                    rate(raw.tx_bytes, prev.tx_bytes, secs),
                    rate(raw.rx_packets, prev.rx_packets, secs),
                    rate(raw.tx_packets, prev.tx_packets, secs),
                )
            } else {
                (0.0, 0.0, 0.0, 0.0)
            };

            stats.push(InterfaceStat {
                name,
                rx_bytes: raw.rx_bytes,
                tx_bytes: raw.tx_bytes,
                rx_packets: raw.rx_packets,
                tx_packets: raw.tx_packets,
                rx_errors: raw.rx_errors,
                tx_errors: raw.tx_errors,
                rx_dropped: raw.rx_dropped,
                tx_dropped: raw.tx_dropped,
                rx_bps,
                tx_bps,
                rx_pps,
                tx_pps,
            });
        }

        self.prev = stats
            .iter()
            .map(|s| {
                (
                    s.name.clone(),
                    RawCounters {
                        rx_bytes: s.rx_bytes,
                        tx_bytes: s.tx_bytes,
                        rx_packets: s.rx_packets,
                        tx_packets: s.tx_packets,
                        rx_errors: s.rx_errors,
                        tx_errors: s.tx_errors,
                        rx_dropped: s.rx_dropped,
                        tx_dropped: s.tx_dropped,
                    },
                )
            })
            .collect();

        stats.sort_by(|a, b| b.rx_bps.total_cmp(&a.rx_bps));
        Ok(stats)
    }
}

fn rate(current: u64, previous: u64, secs: f64) -> f64 {
    current.saturating_sub(previous) as f64 / secs
}

fn read_proc_net_dev() -> Result<HashMap<String, RawCounters>> {
    let text = fs::read_to_string(PROC_NET_DEV)
        .with_context(|| format!("read {PROC_NET_DEV}"))?;
    parse_proc_net_dev(&text)
}

fn parse_proc_net_dev(text: &str) -> Result<HashMap<String, RawCounters>> {
    let mut map = HashMap::new();

    for line in text.lines().skip(2) {
        let line = line.trim();
        if line.is_empty() {
            continue;
        }
        let Some((name, rest)) = line.split_once(':') else {
            continue;
        };
        let name = name.trim().to_string();
        if name.is_empty() {
            continue;
        }

        let fields: Vec<u64> = rest
            .split_whitespace()
            .filter_map(|s| s.parse().ok())
            .collect();
        if fields.len() < 16 {
            continue;
        }

        map.insert(
            name,
            RawCounters {
                rx_bytes: fields[0],
                rx_packets: fields[1],
                rx_errors: fields[2],
                rx_dropped: fields[3],
                tx_bytes: fields[8],
                tx_packets: fields[9],
                tx_errors: fields[10],
                tx_dropped: fields[11],
            },
        );
    }

    Ok(map)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parse_proc_net_dev_sample() {
        let sample = "\
Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
  lo: 12345    100    0    0    0     0          0         0     12345    100    0    0    0     0       0          0
eth0: 999999  5000    1    2    0     0          0         0    888888  4000    3    4    0     0       0          0
";
        let map = parse_proc_net_dev(sample).unwrap();
        assert_eq!(map.len(), 2);
        let lo = map.get("lo").unwrap();
        assert_eq!(lo.rx_bytes, 12345);
        let eth0 = map.get("eth0").unwrap();
        assert_eq!(eth0.tx_dropped, 4);
    }

    #[test]
    fn rate_computes_delta() {
        assert!((rate(2000, 1000, 1.0) - 1000.0).abs() < f64::EPSILON);
    }
}
