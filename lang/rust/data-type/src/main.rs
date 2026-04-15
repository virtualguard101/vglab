fn main() {
    // 1. integer overflow
    let a: u8 = 255;

    // wrapping_add: loop back to 0 after overflow and plus rest part
    let b = a.wrapping_add(6);
    println!("b: {}", b); // 5

    // checked_add: return None if overflow
    let c = a.checked_add(6);
    println!("c: {:?}", c); // None

    // overflowing_add: return the result and the overflow flag
    let (d, overflow) = a.overflowing_add(6);
    println!("d: {}, overflow: {}", d, overflow); // 5, true

    // saturating_add: return the maximum or minimum value if overflow
    let e = a.saturating_add(6);
    println!("e: {}", e); // 255
    assert_eq!(100u8.saturating_add(1), 101);
    assert_eq!(u8::MAX.saturating_add(127), u8::MAX);

    // 2. floating point number
    let _x = 5.0; // f64 by default
    let _y: f32 = 5.0; // f32

    // compare floating point numbers directly (avoid direct comparison in floating point numbers!!!)
    assert!(0.25 + 0.25 == 0.5); // true
    // assert!(0.1 + 0.2 == 0.3); // false
    assert!((0.1_f64 + 0.2 - 0.3).abs() < 0.00001); // true

    // floating point number precision issue 
    let abc: (f32, f32, f32) = (0.1, 0.2, 0.3);
    let xyz: (f64, f64, f64) = (0.1, 0.2, 0.3);
    println!("abc (f32)");
    println!("   0.1 + 0.2: {:x}", (abc.0 + abc.1).to_bits());
    println!("         0.3: {:x}", (abc.2).to_bits());
    println!();
    println!("xyz (f64)");
    println!("   0.1 + 0.2: {:x}", (xyz.0 + xyz.1).to_bits());
    println!("         0.3: {:x}", (xyz.2).to_bits());
    println!();
    assert!(abc.0 + abc.1 == abc.2); // in f32, true because of its lower precision
    // assert!(xyz.0 + xyz.1 == xyz.2); // in f64, false because of its higher precision

    // NaN
    let x = (-42.0_f32).sqrt();
    println!("{}", x);  // NaN
    assert!(x.is_nan());// use `is_nan()` to check if the value is NaN
    // assert!(x == x);    // false
}
