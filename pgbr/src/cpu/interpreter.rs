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
      _ => unreachable!(),
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
            let addr = Self::get_nn(cpu);
            cpu.write16(addr, cpu.reg.get_sp());
          }
          2 => {
            // stop
            todo!("DO--- STOP ME NOW")
          }
          3 => {
            // jr d
            let d = Self::get_d(cpu) as i16;
            let addr = cpu.reg.get_pc().wrapping_add_signed(d);
            cpu.reg.set_pc(addr);
            cpu.add_clock(4);
          }
          4..=7 => {
            // jr cc[y-4], d
            let should_jump = Self::should_jump(cpu, instr.get_y() - 4);
            if should_jump {
              let d = Self::get_d(cpu) as i16;
              let addr = cpu.reg.get_pc().wrapping_add_signed(d);
              cpu.reg.set_pc(addr);
            } else {
              let addr = cpu.reg.get_pc().wrapping_add_signed(1);
              cpu.reg.set_pc(addr);
            }
            cpu.add_clock(4);
          }
          _ => unreachable!(),
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
            let hl = cpu.reg.get_hl();
            let v = Self::get_rp(cpu, instr.get_p());
            let add = hl.wrapping_add(v);
            let half_add = (hl & 0xFFF) + (v & 0xFFF);
            cpu.reg.set_subtract_flag(false);
            cpu.reg.set_half_carry_flag(half_add > 0xFFF);
            cpu.reg.set_carry_flag(add < hl);
            cpu.reg.set_hl(add);
            cpu.add_clock(4);
          }
          _ => unreachable!(),
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
              _ => unreachable!(),
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
              _ => unreachable!(),
            }
          }
          _ => unreachable!(),
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
          _ => unreachable!(),
        }
      }
      4 => {
        // 8 bit increment
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
      5 => {
        // 8 bit decrement
        // dec r[y]
        let r = Self::get_r(cpu, instr.get_y());
        let res = r.wrapping_sub(1);
        let half_res = (r & 0x0F).wrapping_sub(1);
        Self::set_r(cpu, instr.get_y(), res);
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(true);
        // TODO: verify this; it was checking > 0xF0 before, which seems wrong
        cpu.reg.set_half_carry_flag(half_res > 0xF);
      }
      6 => {
        // 8 bit load immediate
        // ld r[y], n
        let v = Self::get_n(cpu);
        Self::set_r(cpu, instr.get_y(), v);
      }
      7 => {
        // misc flag stuff
        match instr.get_y() {
          0 => {
            // rlca
            let a = cpu.reg.get_a();
            cpu.reg.set_zero_flag(false);
            cpu.reg.set_subtract_flag(false);
            cpu.reg.set_half_carry_flag(false);
            cpu.reg.set_carry_flag((a & 0x80) != 0);
            cpu.reg.set_a((a << 1) | (a >> 7));
          }
          1 => {
            // rrca
            let a = cpu.reg.get_a();
            cpu.reg.set_zero_flag(false);
            cpu.reg.set_subtract_flag(false);
            cpu.reg.set_half_carry_flag(false);
            cpu.reg.set_carry_flag((a & 0x1) != 0);
            cpu.reg.set_a(a.rotate_right(1));
          }
          2 => {
            // rla
            let a = cpu.reg.get_a();
            let carry = if cpu.reg.get_carry_flag() { 1 } else { 0 };
            cpu.reg.set_zero_flag(false);
            cpu.reg.set_subtract_flag(false);
            cpu.reg.set_half_carry_flag(false);
            cpu.reg.set_carry_flag((a & 0x80) != 0);
            cpu.reg.set_a((a << 1) | carry);
          }
          3 => {
            // rra
            let a = cpu.reg.get_a();
            let carry = if cpu.reg.get_carry_flag() { 0x80 } else { 0 };
            cpu.reg.set_zero_flag(false);
            cpu.reg.set_subtract_flag(false);
            cpu.reg.set_half_carry_flag(false);
            cpu.reg.set_carry_flag((a & 0x1) != 0);
            cpu.reg.set_a((a >> 1) | carry);
          }
          4 => {
            // daa
            let mut res = cpu.reg.get_a() as i16;
            if cpu.reg.get_subtract_flag() {
              if cpu.reg.get_half_carry_flag() {
                res = res.wrapping_sub(0x06) & 0xFF;
              }
              if cpu.reg.get_carry_flag() {
                res = res.wrapping_sub(0x60);
              }
            } else {
              if cpu.reg.get_half_carry_flag() || (res & 0xF) > 0x9 {
                res = res.wrapping_add(0x06);
              }
              if cpu.reg.get_carry_flag() || res > 0x9F {
                res = res.wrapping_add(0x60);
              }
            }
            cpu.reg.set_half_carry_flag(false);
            cpu.reg.set_zero_flag((res & 0xFF) == 0);
            if res & 0x100 != 0 {
              cpu.reg.set_carry_flag(true);
            }
            cpu.reg.set_a(res as u8);
          }
          5 => {
            // cpl
            cpu.reg.set_a(!cpu.reg.get_a());
            cpu.reg.set_subtract_flag(true);
            cpu.reg.set_half_carry_flag(true);
          }
          6 => {
            // scf
            cpu.reg.set_carry_flag(true);
            cpu.reg.set_half_carry_flag(false);
            cpu.reg.set_subtract_flag(false);
          }
          7 => {
            // ccf
            cpu.reg.set_carry_flag(!cpu.reg.get_carry_flag());
            cpu.reg.set_half_carry_flag(false);
            cpu.reg.set_subtract_flag(false);
          }
          _ => unreachable!(),
        }
      }
      _ => unreachable!(),
    }
  }

  fn interpret_block1(cpu: &mut CPU, instr: GBInstruction) {
    // ld r[y], r[z]
    let y = instr.get_y();
    let z = instr.get_z();
    if y == 6 && z == 6 {
      cpu.halt();
      return;
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
            let should_jump = Self::should_jump(cpu, instr.get_y());
            if should_jump {
              Self::ret(cpu);
              cpu.add_clock(4);
            }
            cpu.add_clock(4);
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
            let add = sp.wrapping_add_signed(off);
            let half_add = (sp & 0xF) + (off & 0xF) as u16;
            let byte_add = (sp & 0xFF).wrapping_add_signed(off & 0xFF);
            cpu.reg.set_zero_flag(false);
            cpu.reg.set_subtract_flag(false);
            cpu.reg.set_half_carry_flag(half_add > 0xF);
            cpu.reg.set_carry_flag(byte_add > 0xFF);
            cpu.reg.set_sp(add);
            cpu.add_clock(8);
          }
          6 => {
            // ldh a, n
            // ld a, (0xFF00 + n)
            let off = Self::get_n(cpu) as u16;
            let v = cpu.read8(0xFF00 | off);
            cpu.reg.set_a(v);
          }
          7 => {
            // ld hl, sp + d
            let off = Self::get_d(cpu) as i16;
            let sp = cpu.reg.get_sp();
            let add = sp.wrapping_add_signed(off);
            let half_add = (sp & 0xF) + (off & 0xF) as u16;
            let byte_add = (sp & 0xFF).wrapping_add_signed(off & 0xFF);
            cpu.reg.set_zero_flag(false);
            cpu.reg.set_subtract_flag(false);
            cpu.reg.set_half_carry_flag(half_add > 0xF);
            cpu.reg.set_carry_flag(byte_add > 0xFF);
            cpu.reg.set_hl(add);
            cpu.add_clock(4);
          }
          _ => unreachable!(),
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
                cpu.add_clock(4);
              }
              1 => {
                // reti
                Self::ret(cpu);
                cpu.add_clock(4);
                cpu.enable_interrupts();
              }
              2 => {
                // jp hl
                let addr = cpu.reg.get_hl();
                cpu.reg.set_pc(addr);
              }
              3 => {
                // ld sp, hl
                cpu.reg.set_sp(cpu.reg.get_hl());
                cpu.add_clock(4);
              }
              _ => unreachable!(),
            }
          }
          _ => unreachable!(),
        }
      }
      2 => {
        // conditional jumps, hi c
        match instr.get_y() {
          0..=3 => {
            // jp cc[y], nn
            let should_jump = Self::should_jump(cpu, instr.get_y());
            if should_jump {
              let addr = Self::get_nn(cpu);
              cpu.reg.set_pc(addr);
            } else {
              let addr = cpu.reg.get_pc().wrapping_add_signed(2);
              cpu.reg.set_pc(addr);
              cpu.add_clock(4);
            }
            cpu.add_clock(4);
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
          _ => unreachable!(),
        }
      }
      3 => {
        // interrupts, jumps, cb prefix
        match instr.get_y() {
          0 => {
            // jp nn
            let addr = cpu.pc_read16();
            cpu.reg.set_pc(addr);
            cpu.add_clock(4);
          }
          1 => {
            // cb prefix
            Self::interpret_cb(cpu);
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
          _ => unreachable!(),
        }
      }
      4 => {
        // conditional call
        match instr.get_y() {
          0..=3 => {
            // call cc[y], nn
            let should_jump = Self::should_jump(cpu, instr.get_y());
            if should_jump {
              let addr = Self::get_nn(cpu);
              Self::call(cpu, addr);
            } else {
              let addr = cpu.reg.get_pc().wrapping_add_signed(2);
              cpu.reg.set_pc(addr);
              cpu.add_clock(4);
            }
            cpu.add_clock(4);
          }
          4..=7 => {
            // invalid
            cpu.freeze()
          }
          _ => unreachable!(),
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
                cpu.add_clock(4);
              }
              1..=3 => {
                // invalid
                cpu.freeze();
              }
              _ => unreachable!(),
            }
          }
          _ => unreachable!(),
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
        cpu.add_clock(4);
      }
      _ => unreachable!(),
    }
  }

  fn interpret_cb(cpu: &mut CPU) {
    let op = GBInstruction(cpu.pc_read8());
    match op.get_x() {
      0 => {
        // roll/shift reg/memory
        let v = Self::get_r(cpu, op.get_z());
        let v = Self::rot(cpu, op.get_y(), v);
        Self::set_r(cpu, op.get_z(), v);
      }
      1 => {
        // test bit
        // bit y, r[z]
        let bit = op.get_y();
        let v = Self::get_r(cpu, op.get_z());
        let mask = 1 << bit;
        cpu.reg.set_zero_flag((v & mask) == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(true);
      }
      2 => {
        // reset bit
        // res y, r[z]
        let bit = op.get_y();
        let v = Self::get_r(cpu, op.get_z());
        let mask = !(1 << bit);
        Self::set_r(cpu, op.get_z(), v & mask);
      }
      3 => {
        // set bit
        // set y, r[z]
        let bit = op.get_y();
        let v = Self::get_r(cpu, op.get_z());
        let mask = 1 << bit;
        Self::set_r(cpu, op.get_z(), v | mask);
      }
      _ => unreachable!(),
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
      6 => cpu.read8(cpu.reg.get_hl()),
      7 => cpu.reg.get_a(),
      _ => unreachable!(),
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
      6 => cpu.write8(cpu.reg.get_hl(), value),
      7 => cpu.reg.set_a(value),
      _ => unreachable!(),
    }
  }

  pub fn get_rp(cpu: &mut CPU, reg: u8) -> u16 {
    match reg {
      0 => cpu.reg.get_bc(),
      1 => cpu.reg.get_de(),
      2 => cpu.reg.get_hl(),
      3 => cpu.reg.get_sp(),
      _ => unreachable!(),
    }
  }

  pub fn set_rp(cpu: &mut CPU, reg: u8, value: u16) {
    match reg {
      0 => cpu.reg.set_bc(value),
      1 => cpu.reg.set_de(value),
      2 => cpu.reg.set_hl(value),
      3 => cpu.reg.set_sp(value),
      _ => unreachable!(),
    }
  }

  pub fn get_rp2(cpu: &mut CPU, reg: u8) -> u16 {
    match reg {
      0 => cpu.reg.get_bc(),
      1 => cpu.reg.get_de(),
      2 => cpu.reg.get_hl(),
      3 => cpu.reg.get_af(),
      _ => unreachable!(),
    }
  }

  pub fn set_rp2(cpu: &mut CPU, reg: u8, value: u16) {
    match reg {
      0 => cpu.reg.set_bc(value),
      1 => cpu.reg.set_de(value),
      2 => cpu.reg.set_hl(value),
      3 => cpu.reg.set_af(value),
      _ => unreachable!(),
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
  fn should_jump(cpu: &mut CPU, cc: u8) -> bool {
    match cc {
      0 if !cpu.reg.get_zero_flag() => true,
      1 if cpu.reg.get_zero_flag() => true,
      2 if !cpu.reg.get_carry_flag() => true,
      3 if cpu.reg.get_carry_flag() => true,
      _ => false,
    }
  }

  fn ret(cpu: &mut CPU) {
    let addr = cpu.read16(cpu.reg.get_sp());
    cpu.reg.add_sp(2);
    cpu.reg.set_pc(addr);
  }

  fn call(cpu: &mut CPU, addr: u16) {
    let pc = cpu.reg.get_pc();
    cpu.reg.set_pc(addr);
    Self::push(cpu, pc);
  }

  pub fn push(cpu: &mut CPU, value: u16) {
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
        let c: u8 = match op {
          1 if cpu.reg.get_carry_flag() => 1,
          _ => 0,
        };
        let sum = a as u16 + v as u16 + c as u16;
        let half_sum = (a & 0xF) + (v & 0xF) + c;
        cpu.reg.set_zero_flag((sum & 0xFF) == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(half_sum > 0xF);
        cpu.reg.set_carry_flag(sum > 0xFF);
        cpu.reg.set_a(sum as u8);
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
        cpu.reg.set_zero_flag((diff & 0xFF) == 0);
        cpu.reg.set_subtract_flag(true);
        cpu.reg.set_half_carry_flag((a & 0xF) < (v & 0xF) + c);
        cpu.reg.set_carry_flag(diff > 0xFF);
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
        let halfv = v & 0xF;
        cpu.reg.set_zero_flag(a == v);
        cpu.reg.set_subtract_flag(true);
        cpu.reg.set_half_carry_flag(halfv > halfa);
        cpu.reg.set_carry_flag(v > a);
      }
      _ => unreachable!(),
    }
  }

  fn rot(cpu: &mut CPU, op: u8, v: u8) -> u8 {
    match op {
      0 => {
        // rlc
        let carry = (v & 0x80) >> 7;
        let res = (v << 1) | carry;
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(false);
        cpu.reg.set_carry_flag(carry != 0);
        res
      }
      1 => {
        // rrc
        let carry = (v & 0x1) << 7;
        let res = (v >> 1) | carry;
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(false);
        cpu.reg.set_carry_flag(carry != 0);
        res
      }
      2 => {
        // rl
        let old_carry = if cpu.reg.get_carry_flag() { 1 } else { 0 };
        let carry = (v & 0x80) != 0;
        let res = (v << 1) | old_carry;
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(false);
        cpu.reg.set_carry_flag(carry);
        res
      }
      3 => {
        // rr
        let old_carry = if cpu.reg.get_carry_flag() { 0x80 } else { 0 };
        let carry = (v & 0x1) != 0;
        let res = (v >> 1) | old_carry;
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(false);
        cpu.reg.set_carry_flag(carry);
        res
      }
      4 => {
        // sla
        let carry = v & 0x80;
        let res = v << 1;
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(false);
        cpu.reg.set_carry_flag(carry != 0);
        res
      }
      5 => {
        // sra
        let carry = v & 0x1;
        let msb = v & 0x80;
        let res = (v >> 1) | msb;
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(false);
        cpu.reg.set_carry_flag(carry != 0);
        res
      }
      6 => {
        // swap
        let high = v & 0xF0;
        let low = v & 0x0F;
        let res = (high >> 4) | (low << 4);
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(false);
        cpu.reg.set_carry_flag(false);
        res
      }
      7 => {
        // srl
        let carry = v & 1;
        let res = v >> 1;
        cpu.reg.set_zero_flag(res == 0);
        cpu.reg.set_subtract_flag(false);
        cpu.reg.set_half_carry_flag(false);
        cpu.reg.set_carry_flag(carry != 0);
        res
      }
      _ => unreachable!(),
    }
  }
}
