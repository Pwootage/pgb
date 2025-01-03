mod cpu;
mod mmu;
mod enums;
mod rom;

#[macro_export]
pub macro_rules! const_assert {
    ($($tt:tt)*) => {
        const _: () = assert!($($tt)*);
    }
}

fn main() {
  println!("Hello, world!");
}
