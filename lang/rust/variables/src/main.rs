struct Struct {
    g: i32
}

fn main() {
    // 1. Mutable and Immutable Variables
    // let x = 5;       // immutable variable
    let mut x = 5; // mutable variable
    // let y = x;
    let _y = x;
    println!("The value of x is: {x}");  // named capture (std::fmt::Display)
    x = 6;
    println!("The value of x is: {}", x);// position placeholder + comma argument (std::fmt::Display)

    // 2. Variable Destructuring
    let (a, mut b): (bool, bool) = (true, false);
    println!("a: {:?}, b: {:?}", a, b);  // Debug placeholder + comma argument (std::fmt::Debug)
    b = true;
    assert_eq!(a, b);

    // Destructuring Assignment
    let (c, d, e, f, g): (i32, i32, i32, i32, _);
    (c, d) = (1, 2);
    [e, .., f, _] = [1, 2, 3, 4, 5]; // _ explicitly ignore the value `5`
    Struct {g, ..} = Struct {g: 5};
    assert_eq!([1, 2, 1, 4, 5], [c, d, e, f, g]);

    // 3. Constant
    const SECONDS_PER_HOUR: u32 = 3600; // constant, distinct with immutable variable
    println!("Seconds per hour: {SECONDS_PER_HOUR}");

    // 4. Shadowing
    let x = 5;
    let x = x + 1; // shadowing, new value assigned to the same variable
    {
        let x = x * 2;
        println!("The value of x in the inner scope is: {x}"); // 12
    }
    println!("The value of x in the outer scope is: {x}"); // 6
}
