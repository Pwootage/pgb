use crate::cpu::registers::GBRegisters;
use crate::cpu::CPU;

pub struct GBInterpreter {}

struct GBInstruction(u8);

impl GBInstruction {
  pub fn get_x(&self) -> u8 {
    (self.0 & 0b11000000) >> 6
  }

  pub fn get_y(&self) -> u8 {
    (self.0 & 0b00111000) >> 3
  }

  pub fn get_z(&self) -> u8 {
    self.0 & 0b00000111
  }

  pub fn get_p(&self) -> u8 {
    (self.get_y() >> 1) & 0b00000011
  }

  pub fn get_q(&self) -> u8 {
    self.get_y() & 0b00000001
  }
}

impl GBInterpreter {
  pub fn interpret_instruction(cpu: &mut CPU, instr: u8) {
    let instr = GBInstruction(instr);
    match instr.get_x() {
      0 => Self::interpret_block0(cpu, instr),
      1 => Self::interpret_block1(cpu, instr),
      2 => Self::interpret_block2(cpu, instr),
      3 => Self::interpret_block3(cpu, instr),
      _ => panic!("invalid x"),
    }
  }

  fn interpret_block0(cpu: &mut CPU, instr: GBInstruction) {
    match instr.get_z() {
      0 => {
        // relative jumps, some misc stuff
        match instr.get_y() {
          0 => {
            // nop
          }
          1 => {
            // ld (nn), sp
            let v = Self::get_nn(cpu);
            cpu.reg.set_sp(v);
          }
          2 => {
            // stop
            todo!("DO--- STOP ME NOW")
          }
          3 => {
            // jr d
            let addr = cpu
              .reg
              .get_pc()
              .wrapping_add_signed(Self::get_d(cpu) as i16);
            Self::jump(cpu, addr);
          }
          4..=7 => {
            // jr cc[y-4], d
            let addr = cpu
              .reg
              .get_pc()
              .wrapping_add_signed(Self::get_d(cpu) as i16);
            Self::jump_cc(cpu, addr, instr.get_y() - 4);
          }
          _ => panic!("invalid y"),
        }
      }
      1 => {
        // 16-bit load immediate, add
        match instr.get_q() {
          0 => {
            // ld rp[p], nn
            Self::set_rp(cpu, instr.get_p(), Self::get_nn(cpu));
          }
          1 => {
            // add hl, rp[p]
            let val = cpu
              .reg
              .get_hl()
              .wrapping_add(Self::get_rp(cpu, instr.get_p()));
            cpu.reg.set_hl(val);
          }
          _ => panic!("invalid q"),
        }
      }
      2 => { // indirect load
        match instr.get_q() {
          0 => {
            match instr.get_p() {
              0 => {
                // ld (bc), a
                cpu.write8(cpu.reg.get_bc(), cpu.reg.get_a());
              }
              1 => {
                // ld (de), a
                cpu.write8(cpu.reg.get_de(), cpu.reg.get_a());
              }
              2 => {
                // ld (hl+), a
                cpu.write8(cpu.reg.get_hl(), cpu.reg.get_a());
                cpu.reg.set_hl(cpu.reg.get_hl().wrapping_add(1));
              }
              3 => {
                // ld (hl-), a
                cpu.write8(cpu.reg.get_hl(), cpu.reg.get_a());
                cpu.reg.set_hl(cpu.reg.get_hl().wrapping_sub(1));
              }
              _ => panic!("invalid p")
            }
          }
          1 => {
            match instr.get_p() {
              0 => {
                // ld a, (bc)
                cpu.reg.set_a(cpu.read8(cpu.reg.get_bc()));
              }
              1 => {
                // ld a, (de)
                cpu.reg.set_a(cpu.read8(cpu.reg.get_de()));
              }
              2 => {
                // ld a, (hl+)
                cpu.reg.set_a(cpu.read8(cpu.reg.get_hl()));
                cpu.reg.set_hl(cpu.reg.get_hl().wrapping_add(1));
              }
              3 => {
                // ld a, (hl-)
                cpu.reg.set_a(cpu.read8(cpu.reg.get_hl()));
                cpu.reg.set_hl(cpu.reg.get_hl().wrapping_sub(1));
              }
              _ => panic!("invalid p")
            }
          }
          _ => panic!("invalid q")
        }
      }
      3 => { // 16 bit increment/decrement
      }
      4 => { // 8 bit increment
      }
      5 => { // 8 bit decrement
      }
      6 => { // 8 bit load immediate
      }
      7 => { // misc flag stuff
      }
      _ => panic!("invalid z"),
    }
  }

  fn interpret_block1(cpu: &mut CPU, instr: GBInstruction) {
    todo!()
  }

  fn interpret_block2(cpu: &mut CPU, instr: GBInstruction) {
    todo!()
  }

  fn interpret_block3(cpu: &mut CPU, instr: GBInstruction) {
    todo!()
  }
}

// operands
impl GBInterpreter {
  pub fn get_r(cpu: &mut CPU, reg: u8) -> u8 {
    match reg {
      0 => cpu.reg.get_b(),
      1 => cpu.reg.get_c(),
      2 => cpu.reg.get_d(),
      3 => cpu.reg.get_e(),
      4 => cpu.reg.get_h(),
      5 => cpu.reg.get_l(),
      6 => cpu.mmu.read8(cpu.reg.get_hl()),
      7 => cpu.reg.get_a(),
      _ => panic!("Invalid value"),
    }
  }

  pub fn set_r(cpu: &mut CPU, reg: u8, value: u8) {
    match reg {
      0 => cpu.reg.set_b(value),
      1 => cpu.reg.set_c(value),
      2 => cpu.reg.set_d(value),
      3 => cpu.reg.set_e(value),
      4 => cpu.reg.set_h(value),
      5 => cpu.reg.set_l(value),
      6 => cpu.mmu.write8(cpu.reg.get_hl(), value),
      7 => cpu.reg.set_a(value),
      _ => panic!("Invalid value"),
    }
  }

  pub fn get_rp(cpu: &mut CPU, reg: u8) -> u16 {
    match reg {
      0 => cpu.reg.get_bc(),
      1 => cpu.reg.get_de(),
      2 => cpu.reg.get_hl(),
      3 => cpu.reg.get_sp(),
      _ => panic!("Invalid reg"),
    }
  }

  pub fn set_rp(cpu: &mut CPU, reg: u8, value: u16) {
    match reg {
      0 => cpu.reg.set_bc(value),
      1 => cpu.reg.set_de(value),
      2 => cpu.reg.set_hl(value),
      3 => cpu.reg.set_sp(value),
      _ => panic!("Invalid reg"),
    }
  }

  pub fn get_rp2(cpu: &mut CPU, reg: u8) -> u16 {
    match reg {
      0 => cpu.reg.get_bc(),
      1 => cpu.reg.get_de(),
      2 => cpu.reg.get_hl(),
      3 => cpu.reg.get_af(),
      _ => panic!("Invalid reg"),
    }
  }

  pub fn set_rp2(cpu: &mut CPU, reg: u8, value: u16) {
    match reg {
      0 => cpu.reg.set_bc(value),
      1 => cpu.reg.set_de(value),
      2 => cpu.reg.set_hl(value),
      3 => cpu.reg.set_af(value),
      _ => panic!("Invalid reg"),
    }
  }

  pub fn get_d(cpu: &mut CPU) -> i8 {
    cpu.pc_read8() as i8
  }

  pub fn get_n(cpu: &mut CPU) -> u8 {
    cpu.pc_read8()
  }

  pub fn get_nn(cpu: &mut CPU) -> u16 {
    cpu.pc_read16()
  }
}

// helpers
impl GBInterpreter {
  fn jump(cpu: &mut CPU, addr: u16) {
    cpu.reg.set_pc(addr);
    cpu.add_clock(4);
  }

  fn jump_cc(cpu: &mut CPU, addr: u16, cc: u8) {
    match cc {
      0 if !cpu.reg.get_zero_flag() => Self::jump(cpu, addr),
      1 if cpu.reg.get_zero_flag() => Self::jump(cpu, addr),
      2 if !cpu.reg.get_carry_flag() => Self::jump(cpu, addr),
      3 if cpu.reg.get_zero_flag() => Self::jump(cpu, addr),
      _ => {
        // don't jump
      }
    }
  }
}
