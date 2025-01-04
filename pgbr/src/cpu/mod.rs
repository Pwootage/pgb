mod initparams;
mod registers;
mod interpreter;

use crate::cpu::initparams::{
  GBA_INIT, GBA_INIT_GBC, GBC_INIT, GBC_INIT_GBC, GBP_INIT, GB_INIT, SGB2_INIT, SGB_INIT,
};
use crate::cpu::interpreter::GBInterpreter;
use crate::cpu::registers::GBRegisters;
use crate::enums::GBModel;
use crate::mmu::MMU;

pub struct CPU {
  mmu: MMU,

  pub reg: GBRegisters,

  clock: u64,
  last_clock_m: u16,
  last_clock_t: u16,
}
// general methods
impl CPU {
  pub fn new(mmu: MMU) -> Self {
    let mut res = CPU {
      mmu,
      reg: GBRegisters::new(),
      clock: 0,
      last_clock_m: 0,
      last_clock_t: 0,
    };

    let init_reg = match res.mmu.model {
      GBModel::GB => GB_INIT,
      GBModel::GBP => GBP_INIT,
      GBModel::SGB => SGB_INIT,
      GBModel::SGB2 => SGB2_INIT,
      GBModel::GBC if res.mmu.gbc_mode => GBC_INIT_GBC,
      GBModel::GBC => GBC_INIT,
      GBModel::GBA | GBModel::GBASP if res.mmu.gbc_mode => GBA_INIT_GBC,
      GBModel::GBA | GBModel::GBASP => GBA_INIT,
    };
    res.reg.init_registers(init_reg);

    res
  }

  pub fn print_state(&self) {
    println!("{}", self.get_state());
  }

  pub fn get_state(&self) -> String {
    // format for gbdoctor:
    // A:00 F:11 B:22 C:33 D:44 E:55 H:66 L:77 SP:8888 PC:9999 PCMEM:AA,BB,CC,DD
    let pc = self.reg.get_pc();
    format!("A:{:02X} F:{:02X} B:{:02X} C:{:02X} D:{:02X} E:{:02X} H:{:02X} L:{:02X} SP:{:04X} PC:{:04X} PCMEM:{:02X},{:02X},{:02X},{:02X}",
              self.reg.get_a(), self.reg.get_f(), self.reg.get_b(), self.reg.get_c(),
              self.reg.get_d(), self.reg.get_e(), self.reg.get_h(), self.reg.get_l(),
              self.reg.get_sp(), pc,
              self.mmu.read8(pc), self.mmu.read8(pc + 1), self.mmu.read8(pc + 2), self.mmu.read8(pc + 3))
  }
}

// clock
impl CPU {
  // clock
  pub fn get_clock(&self) -> u64 {
    self.clock
  }

  pub fn add_clock(&mut self, value: u64) {
    self.clock += value;
  }
}

// memory access
impl CPU {
  pub fn read8(&mut self, addr: u16) -> u8 {
    let v = self.mmu.read8(addr);
    self.add_clock(4);
    v
  }

  pub fn read16(&mut self, addr: u16) -> u16 {
    let v = self.mmu.read16(addr);
    self.add_clock(8);
    v
  }

  pub fn write8(&mut self, addr: u16, value: u8) {
    self.mmu.write8(addr, value);
    self.add_clock(4);
  }

  pub fn write16(&mut self, addr: u16, value: u16) {
    self.mmu.write16(addr, value);
    self.add_clock(8);
  }

  pub fn pc_read8(&mut self) -> u8 {
    let v = self.read8(self.reg.get_pc());
    self.reg.add_pc(1);
    v
  }

  pub fn pc_read16(&mut self) -> u16 {
    let v = self.read16(self.reg.get_pc());
    self.reg.add_pc(2);
    v
  }
}

// emulate instructions
impl CPU {
  pub fn emulate_instruction(&mut self) {
    let op = self.pc_read8();
    // instrUsages[op]++;

    // interpreter::ops[op](this);
    GBInterpreter::interpret_instruction(self, op);

    // self.print_state();
  }

  pub fn enable_interrupts(&mut self) {
    self.mmu.interrupts_enabled = true;
  }

  pub fn disable_interrupts(&mut self) {
    self.mmu.interrupts_enabled = false;
  }

  pub fn freeze(&mut self) {
    // TODO: freeze. Here's the source from gambatte, when I have timers and stuff working
    //	// permanently halt CPU.
    //	// simply halt and clear IE to avoid unhalt from occuring,
    //	// which avoids additional state to represent a "frozen" state.
    //	nontrivial_ff_write(0xFF, 0, cc);
    //	ackDmaReq(intreq_);
    //	intreq_.halt();
    //  if (cc < mem.nextEventTime()) {
    //    unsigned long cycles = mem.nextEventTime() - cc;
    //    cc += cycles + (-cycles & 3);
    //  }
  }
}
