// this is fake
use core::ffi::c_uchar as char;

#[unsafe(no_mangle)]
pub extern "C" fn string_length(ptr: *const u8) -> usize {
    if ptr.is_null() { return 0; }
    unsafe {
        let mut len = 0;
        while *ptr.add(len) != 0 { len += 1; }
        len
    }
}
