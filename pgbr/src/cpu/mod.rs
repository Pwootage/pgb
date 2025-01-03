
const GB_INIT: [u16; 4] = [
  0x01B0,
  0x0013,
  0x00D8,
  0x014D,
];

const GBP_INIT: [u16; 4] = [
  0xffb0,
  0x0013,
  0x00D8,
  0x014D,
];

const GBC_INIT: [u16; 4] = [
  0x1180,
  0x0000,
  0x0008,
  0x007C,
];

const SGB_INIT: [u16; 4] = [
  0x0100,
  0x0014,
  0x0000,
  0xC060,
];

const SGB2_INIT: [u16; 4] = [
  0xFF00,
  0x0000,
  0x0000,
  0x0000,
];

const GBA_INIT: [u16; 4] = [
  0x1100,
  0x0100,
  0x0008,
  0x007C,
];

const GBC_INIT_GBC: [u16; 4] = [
  0x1180,
  0x0000,
  0xFF56,
  0x000D,
];

const BA_INIT_GBC: [u16; 4] = [
  0x1100,
  0x0100,
  0xFF56,
  0x000D,
];

pub struct CPU {
  af: u16,
  bc: u16,
  de: u16,
  hl: u16,
  sp: u16,
  pc: u16,

  clock: u64,
  last_clock_m: u16,
  last_clock_t: u16,
}