use crate::rom::ROM;
use std::collections::{HashMap, HashSet};
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
  // test_rom("../testRoms/basic_tests/build/test.gb");
  // test_rom("../testRoms/blargg/cpu_instrs/individual/01-special.gb");
  // test_rom("../testRoms/blargg/cpu_instrs/individual/02-interrupts.gb");
  // test_rom("../testRoms/blargg/cpu_instrs/individual/03-op sp,hl.gb");
  // test_rom("../testRoms/blargg/cpu_instrs/individual/04-op r,imm.gb");
  // test_rom("../testRoms/blargg/cpu_instrs/individual/05-op rp.gb");
  // test_rom("../testRoms/blargg/cpu_instrs/individual/06-ld r,r.gb");
  // test_rom("../testRoms/blargg/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb");
  // test_rom("../testRoms/blargg/cpu_instrs/individual/08-misc instrs.gb");
  // test_rom("../testRoms/blargg/cpu_instrs/individual/09-op r,r.gb");
  // test_rom("../testRoms/blargg/cpu_instrs/individual/10-bit ops.gb");
  // test_rom("../testRoms/blargg/cpu_instrs/individual/11-op a,(hl).gb");

  test_rom("../testRoms/blargg/cpu_instrs/cpu_instrs.gb");
  test_rom("../testRoms/blargg/instr_timing/instr_timing.gb");
  // test_rom("../testRoms/blargg/interrupt_time/interrupt_time.gb");
  test_rom("../testRoms/blargg/mem_timing/mem_timing.gb");
  // test_rom("../testRoms/blargg/mem_timing-2/mem_timing.gb");
  // test_rom("../testRoms/blargg/mem_timing-2/rom_singles/01-read_timing.gb");
  // test_rom("../testRoms/blargg/halt_bug.gb");
}

fn test_rom(path: &str) {
  println!("Testing rom: {}", path);
  let rom = rom::ROM::new_from_file(path);
  let log = false;
  let timing = false;

  let mmu = mmu::MMU::new(rom);
  let mut cpu = cpu::CPU::new(mmu);

  let log_file = std::fs::File::create("log.txt").unwrap();
  let mut stream = std::io::BufWriter::new(log_file);
  let state = cpu.get_state();
  writeln!(stream, "{}", state).unwrap();

  let mut instr_times: HashMap<u8, HashSet<u64>> = HashMap::new();


  let mut i = 0usize;
  while i < 50_000_000 {
    i += 1;
    let start = cpu.get_clock();

    let op = cpu.emulate_instruction();

    if let Some(op) = op {
      let end = cpu.get_clock();
      let time = (end - start) / 4;
      instr_times.entry(op).or_insert(HashSet::new()).insert(time);
    }

    if log && op.is_some() {
      let state = cpu.get_state();
      writeln!(stream, "{}", state).unwrap();
      i += 1;
    }
  }
  if timing {
    let expected = [
      1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1, 0, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,
      2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1, 2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1,
      1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
      1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 0, 2, 1, 1, 1, 1, 1, 1, 2, 1,
      1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
      1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
      2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 0, 3, 6, 2, 4, 2, 3, 3, 0, 3, 4, 2, 4, 2, 4, 3, 0, 3, 0, 2, 4,
      3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4, 3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4,
    ];
    for i in 0..=255u8 {
      if let Some(times) = instr_times.get(&i) {
        if times.len() > 1 {
          println!("Instruction {:02x} took {:?} cycles", i, times);
        }
      }
    }
    for i in 0..=255u8 {
      if let Some(times) = instr_times.get(&i) {
        let expected_time = expected[i as usize];
        if !times.contains(&expected_time) {
          println!(
            "Instruction {:02x} took {:?} cycles, expected {}",
            i, times, expected_time
          );
        }
      }
    }
  }
  // // print instruction timing table in a 16x16 csv grid
  // for i in 0..256 {
  //   if i % 16 == 0 {
  //     print!("\n");
  //   }
  //   print!("{},", instr_times[i] / 4);
  // }

  stream.flush().unwrap();
}
