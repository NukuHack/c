#![no_std]

mod math;
mod strings;

pub use math::*;
pub use strings::*;

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}
