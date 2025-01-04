mod cpu;
mod enums;
mod mmu;
mod rom;

#[macro_export]
macro_rules! const_assert {
    ($($tt:tt)*) => {
        const _: () = assert!($($tt)*);
    }
}

fn main() {
  let rom = rom::ROM::new_from_file("../testRoms/basic_tests/build/test.gb");
  let mmu = mmu::MMU::new(rom);
  let mut cpu = cpu::CPU::new(mmu);

  for _ in 0..15000 {
    cpu.emulate_instruction();
    // cpu.print_state();
  }
}
