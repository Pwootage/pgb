use std::mem::offset_of;
use crate::const_assert;
use crate::cpu::initparams::InitParams;

pub struct GBRegisters {
  a: u8,
  f: u8,
  b: u8,
  c: u8,
  d: u8,
  e: u8,
  h: u8,
  l: u8,

  sp: u16,
  pc: u16,
}

impl GBRegisters {
  // some verificaiton of aligns for optimization reasons
  const_assert!(offset_of!(GBRegisters, a) == 0);
  const_assert!(offset_of!(GBRegisters, f) == 1);
  const_assert!(offset_of!(GBRegisters, b) == 2);
  const_assert!(offset_of!(GBRegisters, c) == 3);
  const_assert!(offset_of!(GBRegisters, d) == 4);
  const_assert!(offset_of!(GBRegisters, e) == 5);
  const_assert!(offset_of!(GBRegisters, h) == 6);
  const_assert!(offset_of!(GBRegisters, l) == 7);
}

impl GBRegisters {
  pub fn new() -> Self {
    GBRegisters {
      a: 0,
      f: 0,
      b: 0,
      c: 0,
      d: 0,
      e: 0,
      h: 0,
      l: 0,
      sp: 0,
      pc: 0,
    }
  }

  pub fn init_registers(&mut self, init_params: InitParams) {
    self.sp = 0xFFFE;
    self.pc = 0x0100;
    self.a = init_params.a;
    self.f = init_params.f;
    self.b = init_params.b;
    self.c = init_params.c;
    self.d = init_params.d;
    self.e = init_params.e;
    self.h = init_params.h;
    self.l = init_params.l;
  }

  // single registers
  pub fn get_a(&self) -> u8 {
    self.a
  }

  pub fn set_a(&mut self, value: u8) {
    self.a = value;
  }

  pub fn get_f(&self) -> u8 {
    self.f
  }

  pub fn set_f(&mut self, value: u8) {
    self.f = value & 0xF0;
  }

  pub fn get_b(&self) -> u8 {
    self.b
  }

  pub fn set_b(&mut self, value: u8) {
    self.b = value;
  }

  pub fn get_c(&self) -> u8 {
    self.c
  }

  pub fn set_c(&mut self, value: u8) {
    self.c = value;
  }

  pub fn get_d(&self) -> u8 {
    self.d
  }

  pub fn set_d(&mut self, value: u8) {
    self.d = value;
  }

  pub fn get_e(&self) -> u8 {
    self.e
  }

  pub fn set_e(&mut self, value: u8) {
    self.3 = value;
  }

  pub fn get_h(&self) -> u8 {
    self.h
  }

  pub fn set_h(&mut self, value: u8) {
    self.h = value;
  }

  pub fn get_l(&self) -> u8 {
    self.l
  }

  pub fn set_l(&mut self, value: u8) {
    self.l = value;
  }

  // combo registers
  pub fn get_af(&self) -> u16 {
    (self.get_a() as u16) << 8 | self.get_f() as u16
  }

  pub fn set_af(&mut self, value: u16) {
    self.set_a((value >> 8) as u8);
    self.set_f(value as u8);
  }

  pub fn get_bc(&self) -> u16 {
    (self.get_b() as u16) << 8 | self.get_c() as u16
  }

  pub fn set_bc(&mut self, value: u16) {
    self.set_b((value >> 8) as u8);
    self.set_c(value as u8);
  }

  pub fn get_de(&self) -> u16 {
    (self.get_d() as u16) << 8 | self.get_e() as u16
  }

  pub fn set_de(&mut self, value: u16) {
    self.set_d((value >> 8) as u8);
    self.set_e(value as u8);
  }

  pub fn get_hl(&self) -> u16 {
    (self.get_h() as u16) << 8 | self.get_l() as u16
  }

  pub fn set_hl(&mut self, value: u16) {
    self.set_h((value >> 8) as u8);
    self.set_l(value as u8);
  }

  // pc/sp
  pub fn get_sp(&self) -> u16 {
    self.sp
  }

  pub fn set_sp(&mut self, value: u16) {
    self.sp = value;
  }

  pub fn add_sp(&mut self, v: i16) {
    self.sp = self.sp.wrapping_add_signed(v);
  }

  pub fn get_pc(&self) -> u16 {
    self.pc
  }

  pub fn set_pc(&mut self, value: u16) {
    self.pc = value;
  }

  pub fn add_pc(&mut self, value: u16) {
    self.pc = self.pc.wrapping_add(value);
  }

  // flags
  pub fn get_zero_flag(&self) -> bool {
    self.f & 0x80 != 0
  }

  pub fn set_zero_flag(&mut self, value: bool) {
    if value {
      self.f |= 0x80;
    } else {
      self.f &= 0x7F;
    }
  }

  pub fn get_subtract_flag(&self) -> bool {
    self.f & 0x40 != 0
  }

  pub fn set_subtract_flag(&mut self, value: bool) {
    if value {
      self.f |= 0x40;
    } else {
      self.f &= 0xBF;
    }
  }

  pub fn get_half_carry_flag(&self) -> bool {
    self.f & 0x20 != 0
  }

  pub fn set_half_carry_flag(&mut self, value: bool) {
    if value {
      self.f |= 0x20;
    } else {
      self.f &= 0xDF;
    }
  }

  pub fn get_carry_flag(&self) -> bool {
    self.f & 0x10 != 0
  }

  pub fn set_carry_flag(&mut self, value: bool) {
    if value {
      self.f |= 0x10;
    } else {
      self.f &= 0xEF;
    }
  }
}