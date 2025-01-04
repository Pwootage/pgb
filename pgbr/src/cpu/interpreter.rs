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
            let d = Self::get_d(cpu) as i16;
            let addr = cpu.reg.get_pc().wrapping_add_signed(d);
            Self::jump(cpu, addr);
          }
          4..=7 => {
            // jr cc[y-4], d
            let addr = | cpu: &mut CPU | {
              let d = Self::get_d(cpu) as i16;
              cpu.reg.get_pc().wrapping_add_signed(d)
            };
            Self::jump_cc(cpu, instr.get_y() - 4, addr, 1);
          }
          _ => panic!("invalid y"),
        }
      }
      1 => {
        // 16-bit load immediate, add
        match instr.get_q() {
          0 => {
            // ld rp[p], nn
            let v = Self::get_nn(cpu);
            Self::set_rp(cpu, instr.get_p(), v);
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
      2 => {
        // indirect load
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
              _ => panic!("invalid p"),
            }
          }
          1 => {
            match instr.get_p() {
              0 => {
                // ld a, (bc)
                let v = cpu.read8(cpu.reg.get_bc());
                cpu.reg.set_a(v);
              }
              1 => {
                // ld a, (de)
                let v = cpu.read8(cpu.reg.get_de());
                cpu.reg.set_a(v);
              }
              2 => {
                // ld a, (hl+)
                let v = cpu.read8(cpu.reg.get_hl());
                cpu.reg.set_a(v);
                cpu.reg.set_hl(cpu.reg.get_hl().wrapping_add(1));
              }
              3 => {
                // ld a, (hl-)
                let v = cpu.read8(cpu.reg.get_hl());
                cpu.reg.set_a(v);
                cpu.reg.set_hl(cpu.reg.get_hl().wrapping_sub(1));
              }
              _ => panic!("invalid p"),
            }
          }
          _ => panic!("invalid q"),
        }
      }
      3 => {
        // 16 bit increment/decrement
        match instr.get_q() {
          0 => {
            // inc rp[p]
            let r = Self::get_rp(cpu, instr.get_p());
            Self::set_rp(cpu, instr.get_p(), r.wrapping_add(1));
            cpu.add_clock(4);
          }
          1 => {
            // dec rp[p]
            let r = Self::get_rp(cpu, instr.get_p());
            Self::set_rp(cpu, instr.get_p(), r.wrapping_sub(1));
            cpu.add_clock(4);
          }
          _ => panic!("invalid q"),
        }
      }
      4 => { // 8 bit increment
        // inc r[y]
        let r = Self::get_r(cpu, instr.get_y());
        let res = r.wrapping_add(1);
        let half_res = (r & 0x0F).wrapping_add(1);
        Self::set_r(cpu, instr.get_y(), res);
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        // TODO: verify this; it was checking > 0xF0 before, which seems wrong
        cpu.reg.set_half_carry_flag(half_res > 0xF);
      }
      5 => { // 8 bit decrement
        // dec r[y]
        let r = Self::get_r(cpu, instr.get_y());
        let res = r.wrapping_sub(1);
        let half_res = (r & 0x0F).wrapping_sub(1);
        Self::set_r(cpu, instr.get_y(), res);
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        // TODO: verify this; it was checking > 0xF0 before, which seems wrong
        cpu.reg.set_half_carry_flag(half_res > 0xF);
      }
      6 => { // 8 bit load immediate
        // ld r[y], n
        let v = Self::get_n(cpu);
        Self::set_r(cpu, instr.get_y(), v);
      }
      7 => { // misc flag stuff
        todo!("uhoh, need to do flags")
      }
      _ => panic!("invalid z"),
    }
  }

  fn interpret_block1(cpu: &mut CPU, instr: GBInstruction) {
    // ld r[y], r[z]
    let y = instr.get_y();
    let z = instr.get_z();
    if y == 6 && z == 6 {
      todo!("HALT IN THE NAME OF LOVE")
    }
    let v = Self::get_r(cpu, z);
    Self::set_r(cpu, y, v);
  }

  fn interpret_block2(cpu: &mut CPU, instr: GBInstruction) {
    // alu reg reg
    let value = Self::get_r(cpu, instr.get_z());
    Self::alu(cpu, instr.get_y(), value);
  }

  fn interpret_block3(cpu: &mut CPU, instr: GBInstruction) {
    match instr.get_z() {
      0 => {
        // conditional return, mem mapped register, stack
        match instr.get_y() {
          0..=3 => {
            // ret cc[y]
            Self::ret_cc(cpu, instr.get_y());
          }
          4 => {
            // ldh n, a
            // ld (0xFF00 + n), a
            let off = Self::get_n(cpu) as u16;
            cpu.write8(0xFF00 | off, cpu.reg.get_a());
          }
          5 => {
            // add sp, d
            let off = Self::get_d(cpu) as i16;
            let sp = cpu.reg.get_sp();
            let (add, overflow) = sp.overflowing_add_signed(off);
            let half_add = (sp & 0xFF) + (off & 0xFF) as u16;
            cpu.reg.set_zero_flag(false);
            cpu.reg.set_subtract_flag(false);
            cpu.reg.set_half_carry_flag(half_add > 0xFF);
            cpu.reg.set_carry_flag(overflow);
            cpu.reg.set_sp(add);
          }
          6 => {
            // ld a, (0xFF00 + n)
            let off = Self::get_n(cpu) as u16;
            let v = cpu.read8(0xFF00 | off);
            cpu.reg.set_a(v);
          }
          7 => {
            // ld hl, sp + d
            let off = Self::get_d(cpu) as i16;
            let sp = cpu.reg.get_sp();
            let (add, overflow) = sp.overflowing_add_signed(off);
            let half_add = (sp & 0xFF) + (off & 0xFF) as u16;
            cpu.reg.set_zero_flag(false);
            cpu.reg.set_subtract_flag(false);
            cpu.reg.set_half_carry_flag(half_add > 0xFF);
            cpu.reg.set_carry_flag(overflow);
            cpu.reg.set_hl(add);
          }
          _ => panic!("invalid y")
        }
      }
      1 => {
        // pop/ret/etc
        match instr.get_q() {
          0 => {
            // pop rp2[p]
            let v = cpu.read16(cpu.reg.get_sp());
            Self::set_rp2(cpu, instr.get_p(), v);
            cpu.reg.add_sp(2);
          }
          1 => {
            match instr.get_p() {
              0 => {
                // ret
                Self::ret(cpu);
              }
              1 => {
                // reti
                Self::ret(cpu);
                cpu.enable_interrupts();
              }
              2 => {
                // jp hl
                // TODO: check if this takes an extra clock cycle or not!
                Self::jump(cpu, cpu.reg.get_hl());
              }
              3 => {
                // ld sp, hl
                cpu.reg.set_sp(cpu.reg.get_hl());
                cpu.add_clock(4);
              }
              _ => panic!("invalid p")
            }
          }
          _ => panic!("invalid q")
        }
      }
      2 => {
        // conditional jumps, hi c
        match instr.get_y() {
          0..=3 => {
            // jp cc[y], nn
            let addr = | cpu: &mut CPU | Self::get_nn(cpu);
            Self::jump_cc(cpu, instr.get_y(), addr, 2);
          }
          4 => {
            // ld (0xFF00+c), a
            let off = cpu.reg.get_c() as u16;
            cpu.write8(0xFF00 | off, cpu.reg.get_a());
          }
          5 => {
            // ld (nn), a
            let off = Self::get_nn(cpu);
            cpu.write8(off, cpu.reg.get_a());
          }
          6 => {
            // ld a, (0xFF00+c)
            let off = cpu.reg.get_c() as u16;
            let v = cpu.read8(0xFF00 | off);
            cpu.reg.set_a(v);
          }
          7 => {
            // ld a, (nn)
            let off = Self::get_nn(cpu);
            let v = cpu.read8(off);
            cpu.reg.set_a(v);
          }
          _ => panic!("invalid y")
        }
      }
      3 => {
        // interrupts, jumps, cb prefix
        match instr.get_y() {
          0 => {
            // jp nn
            let addr = cpu.pc_read16();
            Self::jump(cpu, addr);
          }
          1 => {
            // cb prefix
            todo!("no cb yet");
          }
          2 => {
            // invalid
            cpu.freeze();
          }
          3 => {
            // invalid
            cpu.freeze();
          }
          4 => {
            // invalid
            cpu.freeze();
          }
          5 => {
            // invalid
            cpu.freeze();
          }
          6 => {
            // di
            cpu.disable_interrupts();
          }
          7 => {
            // ei
            cpu.enable_interrupts();
          }
          _ => panic!("invalid y")
        }
      }
      4 => {
        // conditional call
        match instr.get_y() {
          0..=3 => {
            // call cc[y], nn
            let addr = | cpu: &mut CPU | Self::get_nn(cpu);
            Self::call_cc(cpu, instr.get_y(), addr, 1);
          }
          4..=7 => {
            // invalid
            cpu.freeze()
          }
          _ => panic!("invalid y")
        }
      }
      5 => {
        // push, unconditional call
        match instr.get_q() {
          0 => {
            // push rp2[p]
            let v = Self::get_rp2(cpu, instr.get_p());
            cpu.add_clock(4);
            Self::push(cpu, v);
          }
          1 => {
            match instr.get_p() {
              0 => {
                // call nn
                let addr = Self::get_nn(cpu);
                Self::call(cpu, addr);
              }
              1..=3 => {
                // invalid
                cpu.freeze();
              }
              _ => panic!("invalid p")
            }
          }
          _ => panic!("invalid q")
        }
      }
      6 => {
        // alu[y] n
        let n = Self::get_n(cpu);
        Self::alu(cpu, instr.get_y(), n);
      }
      7 => {
        // rst
        Self::push(cpu, cpu.reg.get_pc());
        cpu.reg.set_pc((instr.get_y() * 8) as u16);
      }
      _ => panic!("invalid z")
    }
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

  fn jump_cc(cpu: &mut CPU, cc: u8, addr: fn(&mut CPU) -> u16, skip_on_false: u16) {
    match cc {
      0 if !cpu.reg.get_zero_flag() => {
        let v = addr(cpu);
        Self::jump(cpu, v)
      },
      1 if cpu.reg.get_zero_flag() => {
        let v = addr(cpu);
        Self::jump(cpu, v)
      },
      2 if !cpu.reg.get_carry_flag() => {
        let v = addr(cpu);
        Self::jump(cpu, v)
      },
      3 if cpu.reg.get_zero_flag() => {
        let v = addr(cpu);
        Self::jump(cpu, v)
      },
      _ => {
        // don't jump
        Self::jump(cpu, cpu.reg.get_pc().wrapping_add(skip_on_false))
      }
    }
  }

  fn ret(cpu: &mut CPU) {
    let addr = cpu.read16(cpu.reg.get_sp());
    cpu.reg.add_sp(2);
    Self::jump(cpu, addr);
  }

  fn ret_cc(cpu: &mut CPU,cc: u8) {
    cpu.add_clock(4);
    match cc {
      0 if !cpu.reg.get_zero_flag() => Self::ret(cpu),
      1 if cpu.reg.get_zero_flag() => Self::ret(cpu),
      2 if !cpu.reg.get_carry_flag() => Self::ret(cpu),
      3 if cpu.reg.get_zero_flag() => Self::ret(cpu),
      _ => {
        // don't jump
      }
    }
  }

  fn call(cpu: &mut CPU, addr: u16) {
    let pc = cpu.reg.get_pc();
    Self::jump(cpu, addr);
    Self::push(cpu, pc);
  }

  fn call_cc(cpu: &mut CPU, cc: u8, addr: fn(&mut CPU) -> u16, skip_on_false: u16) {
    match cc {
      0 if !cpu.reg.get_zero_flag() => {
        let v = addr(cpu);
        Self::call(cpu, v)
      },
      1 if cpu.reg.get_zero_flag() => {
        let v = addr(cpu);
        Self::call(cpu, v)
      },
      2 if !cpu.reg.get_carry_flag() => {
        let v = addr(cpu);
        Self::call(cpu, v)
      },
      3 if cpu.reg.get_zero_flag() => {
        let v = addr(cpu);
        Self::call(cpu, v)
      },
      _ => {
        // don't jump
        Self::jump(cpu, cpu.reg.get_pc().wrapping_add(skip_on_false))
      }
    }
  }

  fn push(cpu: &mut CPU, value: u16) {
    cpu.write16(cpu.reg.get_sp().wrapping_sub(2), value);
    cpu.reg.add_sp(-2);
  }

  fn alu(cpu: &mut CPU, op: u8, value: u8) {
    let a = cpu.reg.get_a();
    let v = value;
    match op {
      0 | 1 => {
        // 0: add a,
        // 1: adc a,
        // 2: sub
        // 2: sub a,
        // 3: sbc a,
        let c: u8 = match op {
          1 if cpu.reg.get_carry_flag() => 1,
          _ => 0,
        };
        let sum = a + v + c;
        let half_sum = (a & 0xF) + (v & 0xF) + c;
        cpu.reg.set_zero_flag(sum == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(half_sum > 0xF);
        cpu.reg.set_carry_flag(sum < a - c); // TODO: verify this
        cpu.reg.set_a(sum);
      }
      2 | 3 => {
        // 2: sub
        // 2: sub a,
        // 3: sbc a,
        let c: u8 = match op {
          3 if cpu.reg.get_carry_flag() => 1,
          _ => 0,
        };
        let diff = (a as u16).wrapping_sub(v as u16).wrapping_sub(c as u16);
        let halfa = a & 0xF;
        let halfv = v * 0xF;
        cpu.reg.set_zero_flag(diff == 0);
        cpu.reg.set_subtract_flag(true);
        cpu.reg.set_half_carry_flag(halfv > halfa + c);
        cpu.reg.set_carry_flag(v > a + c);
        cpu.reg.set_a(diff as u8);
      }
      4 => {
        // and
        // and a,
        let res = a & v;
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(true);
        cpu.reg.set_carry_flag(false);
        cpu.reg.set_a(res);
      }
      5 => {
        // xor
        // xor a,
        let res = a ^ v;
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(false);
        cpu.reg.set_carry_flag(false);
        cpu.reg.set_a(res);
      }
      6 => {
        // or
        // or a,
        let res = a | v;
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(false);
        cpu.reg.set_carry_flag(false);
        cpu.reg.set_a(res);
      }
      7 => {
        // cp
        // cp a,
        let halfa = a & 0xF;
        let halfv = v.wrapping_mul(0xF);
        cpu.reg.set_zero_flag(a == v);
        cpu.reg.set_subtract_flag(true);
        cpu.reg.set_half_carry_flag(halfv > halfa);
        cpu.reg.set_carry_flag(v > a);
      }
      _ => panic!("bad alu op")
    }

  }
}
