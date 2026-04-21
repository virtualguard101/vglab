fn main() {
    // 1. statement and expression
    let result = sum_with_add_five(20, 20);
    // let b = (let a = result + 5);
    println!("The result of sum_with_add_five(20, 20): {}", result);

    // assign a statement block
    let b = {
        let x = 5;
        x + 5 // expression in last line
    };
    println!("{}", b);

    // let v = {
    //     let mut x = 1;
    //     x += 4
    // };
    // assert_eq!(v, ());
    let v = {
        let x = 1;
        x + 4
    };
    assert_eq!(v, 5);
    println!();

    // prime numbers indentify
    println!("2 is prime number: {}", is_prime(2));
    println!("5 is prime number: {}", is_prime(5));
    println!("25 is prime number: {}", is_prime(25));
    println!("101 is prime number: {}", is_prime(101));
    println!();

    let _res = sum(2, 3);
    
    dead_end();
}

fn sum(a: i32, b: i32) -> i32 {
    a + b
}

fn sum_with_add_five(a: i32, b: i32) -> i32 {
    let a = a + 5; // statement
    let b = b + 5; // statement
    a + b // expression
}

fn is_prime(num: u32) -> bool {
    for n in 2..num {
        if num % n == 0 {
            return false;
        }
    }
    true
}

fn dead_end() -> ! {
    panic!("You should never see this line.");
}

fn _indentify_loop() -> ! {
    loop {

    }
}
