use core::ffi::c_int as int;


#[unsafe(no_mangle)]
pub extern "C" fn add_numbers(a: int, b: int) -> int {
    a + b
}

#[unsafe(no_mangle)]
pub extern "C" fn multiply_numbers(a: int, b: int) -> int {
    a * b
}
