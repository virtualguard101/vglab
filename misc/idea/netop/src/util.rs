pub fn format_rate(bps: f64) -> String {
    const K: f64 = 1024.0;
    const M: f64 = K * K;
    const G: f64 = M * K;

    if bps >= G {
        format!("{:.2} GB/s", bps / G)
    } else if bps >= M {
        format!("{:.2} MB/s", bps / M)
    } else if bps >= K {
        format!("{:.1} KB/s", bps / K)
    } else {
        format!("{:.0} B/s", bps)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn format_rate_scales() {
        assert_eq!(format_rate(512.0), "512 B/s");
        assert_eq!(format_rate(2048.0), "2.0 KB/s");
    }
}
