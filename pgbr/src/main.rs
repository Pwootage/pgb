use std::io::Write;

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
  let log = false;

  // let rom = rom::ROM::new_from_file("../testRoms/basic_tests/build/test.gb");
  let rom = rom::ROM::new_from_file("../testRoms/blargg/cpu_instrs/cpu_instrs.gb");
  // let rom = rom::ROM::new_from_file("../testRoms/blargg/cpu_instrs/individual/01-special.gb");
  // let rom = rom::ROM::new_from_file("../testRoms/blargg/cpu_instrs/individual/02-interrupts.gb");
  // let rom = rom::ROM::new_from_file("../testRoms/blargg/cpu_instrs/individual/03-op sp,hl.gb");
  // let rom = rom::ROM::new_from_file("../testRoms/blargg/cpu_instrs/individual/04-op r,imm.gb");
  // let rom = rom::ROM::new_from_file("../testRoms/blargg/cpu_instrs/individual/05-op rp.gb");
  // let rom = rom::ROM::new_from_file("../testRoms/blargg/cpu_instrs/individual/06-ld r,r.gb");
  // let rom = rom::ROM::new_from_file("../testRoms/blargg/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb");
  // let rom = rom::ROM::new_from_file("../testRoms/blargg/cpu_instrs/individual/08-misc instrs.gb");
  // let rom = rom::ROM::new_from_file("../testRoms/blargg/cpu_instrs/individual/09-op r,r.gb");
  // let rom = rom::ROM::new_from_file("../testRoms/blargg/cpu_instrs/individual/10-bit ops.gb");
  // let rom = rom::ROM::new_from_file("../testRoms/blargg/cpu_instrs/individual/11-op a,(hl).gb");
  let mmu = mmu::MMU::new(rom);
  let mut cpu = cpu::CPU::new(mmu);

  let log_file = std::fs::File::create("log.txt").unwrap();
  let mut stream = std::io::BufWriter::new(log_file);
  let state = cpu.get_state();
  writeln!(stream, "{}", state).unwrap();

  for i in 1..=100_000_000 {
    // if (i % 1_000_000) == 0 {
    //   println!("{}M", i / 1_000_000);
    // }
    // if cpu.reg.get_pc() == 0x020C {
    //   print!("breakpoint");
    // }
    cpu.emulate_instruction();
    if log {
      let state = cpu.get_state();
      writeln!(stream, "{}", state).unwrap();
    }
  }
  stream.flush().unwrap();
}
