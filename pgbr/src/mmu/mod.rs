mod memorymap;

use crate::enums::{GBModel, GpuMode};
use crate::rom::ROM;
use bitflags::bitflags;
use memorymap::MemoryMap;

// our memory map entries
type ROM0 = MemoryMap<0x0000, 0x3FFF>;
type ROMX = MemoryMap<0x4000, 0x7FFF>;
type VRAM = MemoryMap<0x8000, 0x9FFF>;
type SRAM = MemoryMap<0xA000, 0xBFFF>;
type WRAM0 = MemoryMap<0xC000, 0xCFFF>;
type WRAMX = MemoryMap<0xD000, 0xDFFF>;
type ECHO = MemoryMap<0xE000, 0xFDFF>;
type OAM = MemoryMap<0xFE00, 0xFE9F>;
type UNUSED = MemoryMap<0xFEA0, 0xFEFF>;
type IO = MemoryMap<0xFF00, 0xFF7F>;
type HRAM = MemoryMap<0xFF80, 0xFFFE>;
type IE = MemoryMap<0xFFFF, 0xFFFF>;

bitflags! {
    #[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
    struct LCDControlFlags: u8 {
        const POWER = 0b10000000;
        const WINDOW_TILES = 0b01000000;
        const WINDOW_ENABLE = 0b00100000;
        const BG_WINDOW_TILESET = 0b00010000;
        const BG_TILEMAP = 0b00001000;
        const SPRITE_SIZE = 0b00000100;
        const SPRITE_ENABLE = 0b00000010;
        const BG_ENABLE = 0b00000001;
        const _ = !0; // all bits, although all bits are already defined
    }
}

pub struct MMU {
  rom: ROM,

  // working ram (banked on gbc)
  wram_bank: u8,
  wram: [[u8; VRAM::size()]; 8],
  // video ram (banked on gbc)
  vram_bank: u8,
  vram: [[u8; VRAM::size()]; 2],
  // cpu ram/high ram
  hram: [u8; HRAM::size()],
  // object attribute memory
  oam: [u8; OAM::size()],

  // other flags
  // TODO: bitflag?
  pub interrupt_enable: u8,               // interrupt enable register
  pub interrupts_enabled: bool,           // are interrupts enabled
  pub model: GBModel,                     // what kind of GB is this
  pub gbc_mode: bool,                     // are we in gbc mode
  unused_memory_duplicate_mode: bool, // Weird unused echo memory, TODO: where is this from
  sram_enabled: bool,                 // is sram enabled
  cart_inserted: bool,                // is a cartridge inserted

  // gpu state, since it affects memory readability. Maybe can move to a separate struct?
  gpu_mode: GpuMode,
  gpu_line: u8,
  scroll_x: u8,
  scroll_y: u8,
  lcd_control: LCDControlFlags,
  lyc_check_enable: bool,
  lyc_compare: u8,
  mode2_oam_check_enable: bool,
  mode1_vblank_check_enable: bool,
  mode0_hblank_check_enable: bool,
}

impl MMU {
  pub fn new(rom: ROM) -> Self {
    MMU {
      rom,
      // working ram
      wram_bank: 1,
      wram: [[0; VRAM::size()]; 8],
      // video ram
      vram_bank: 1,
      vram: [[0; VRAM::size()]; 2],
      // high ram
      hram: [0; HRAM::size()],
      // object attribute memory
      oam: [0; OAM::size()],
      // other flags
      interrupt_enable: 0,
      interrupts_enabled: false,
      model: GBModel::GB,
      gbc_mode: false,
      unused_memory_duplicate_mode: false,
      sram_enabled: true,
      cart_inserted: true,
      // gpu state
      gpu_mode: GpuMode::ScanOAM,
      gpu_line: 0,
      scroll_x: 0,
      scroll_y: 0,
      lcd_control: LCDControlFlags::POWER,
      lyc_check_enable: false,
      lyc_compare: 0,
      mode2_oam_check_enable: false,
      mode1_vblank_check_enable: false,
      mode0_hblank_check_enable: false,
    }
  }

  pub fn read8(&self, addr: u16) -> u8 {
    // you can't use match statements with const fn, so we have to use if/else
    if ROM0::contains(addr) {
      if !self.cart_inserted {
        0xFF
      } else {
        self.rom.read(addr)
      }
    } else if ROMX::contains(addr) {
      if !self.cart_inserted {
        0xFF
      } else {
        self.rom.read(addr)
      }
    } else if VRAM::contains(addr) {
      if self.lcd_control.contains(LCDControlFlags::POWER) && self.gpu_mode == GpuMode::ScanVRAM {
        0xFF
      } else {
        if self.model.has_banked_ram() {
          self.vram[self.vram_bank as usize][(addr - VRAM::start()) as usize]
        } else {
          self.vram[0][(addr - VRAM::start()) as usize]
        }
      }
    } else if SRAM::contains(addr) {
      if !self.sram_enabled {
        0xFF
      } else {
        self.rom.read_sram(addr - SRAM::start())
      }
    } else if WRAM0::contains(addr) {
      self.wram[0][(addr - WRAM0::start()) as usize % self.wram[0].len()]
    } else if WRAMX::contains(addr) {
      let bank = if self.model.has_banked_ram() {
        self.wram_bank
      } else {
        1
      };
      self.wram[bank as usize % self.wram.len()][(addr - WRAMX::start()) as usize % self.wram[0].len()]
    } else if ECHO::contains(addr) {
      // Weird unused echo memory
      if self.unused_memory_duplicate_mode {
        let wram = self.read8(addr - ECHO::start() + WRAM0::start());
        let sram = self.read8(addr - ECHO::start() + SRAM::start());
        wram & sram
      } else {
        self.read8(addr - ECHO::start() + WRAM0::start())
      }
    } else if OAM::contains(addr) {
      // Object attribute table (sprite info table)
      let off = addr - OAM::start();
      if off as usize >= self.oam.len() {
        0xFF
      } else if self.lcd_control.contains(LCDControlFlags::POWER) && (self.gpu_mode == GpuMode::ScanOAM || self.gpu_mode == GpuMode::ScanVRAM) {
        0xFF
      } else {
        self.oam[off as usize]
      }
    } else if UNUSED::contains(addr) {
      //TODO unused area weirdness depending on mode
      0
    } else if IO::contains(addr) {
      self.ioread(addr)
    } else if HRAM::contains(addr) {
      return self.hram[(addr - HRAM::start()) as usize];
    } else if IE::contains(addr) {
      self.interrupt_enable
    } else {
      // this is a programmer error
      panic!("Invalid memory read at address {:#06x}", addr);
    }
  }

  pub fn write8(&mut self, addr: u16, value: u8) {
    if ROM0::contains(addr) {
      if self.cart_inserted {
        self.rom.write(addr, value);
      }
    } else if ROMX::contains(addr) {
      //ROMX
      if self.cart_inserted {
        self.rom.write(addr, value);
      }
    } else if VRAM::contains(addr) {
      //VRAM
      if !self.lcd_control.contains(LCDControlFlags::POWER) || self.gpu_mode != GpuMode::ScanVRAM {
        self.vram[(self.vram_bank & 1) as usize][(addr - VRAM::start()) as usize] = value;
      }
    } else if SRAM::contains(addr) {
      //SRAM
      if self.sram_enabled {
        self.rom.write_sram(addr - SRAM::start(), value);
      }
    } else if WRAM0::contains(addr) {
      //WRAM0
      self.wram[0][(addr - WRAMX::start()) as usize % self.wram[0].len()] = value;
    } else if WRAMX::contains(addr) {
      //WRAMX
      let bank = if self.model.has_banked_ram() {
        self.wram_bank
      } else {
        1
      };
      self.wram[bank as usize % self.wram.len()][(addr - WRAMX::start()) as usize % self.wram[0].len()] = value;
    } else if ECHO::contains(addr) {
      // Weird mirror unused memory
      if self.unused_memory_duplicate_mode {
        self.write8(addr - ECHO::start() + WRAM0::start(), value);
        self.write8(addr - ECHO::start() + SRAM::start(), value);
      } else {
        self.write8(addr - ECHO::start() + WRAM0::start(), value);
      }
    } else if OAM::contains(addr) {
      let off = addr - OAM::start();
      if off < self.oam.len() as u16 {
        if !self.lcd_control.contains(LCDControlFlags::POWER) || (self.gpu_mode != GpuMode::ScanOAM && self.gpu_mode != GpuMode::ScanVRAM) {
          self.oam[off as usize] = value;
        }
      }
    } else if UNUSED::contains(addr) {
      //TODO unused area weirdness depending on mode
      // for now, ignore
    } else if IO::contains(addr) {
      // IO registers
      self.iowrite(addr, value);
    } else if HRAM::contains(addr) {
      // Internal CPU ram
      self.hram[(addr - HRAM::start()) as usize] = value;
    } else if IE::contains(addr) {
      self.interrupt_enable = value;
    } else {
      // this is a programmer error
      panic!("Invalid memory write at address {:#06x}", addr);
    }
  }

  pub fn read16(&self, addr: u16) -> u16 {
    self.read8(addr) as u16 | (self.read8(addr + 1) as u16) << 8
  }

  pub fn write16(&mut self, addr: u16, value: u16) {
    self.write8(addr, value as u8);
    self.write8(addr + 1, (value >> 8) as u8);
  }

  fn iowrite(&mut self, addr: u16, value: u8) {
    match addr {
      0xFF01 => { // serial read/write sb
        print!("{}", value as char);
      }
      0xFF40 => { // LCD/GPU control
        self.lcd_control = LCDControlFlags::from_bits_retain(value);
      }
      0xFF41 => { // LCD Status/STAT
        self.lyc_check_enable = (addr & 0x40) > 0;
        self.mode2_oam_check_enable = (addr & 0x20) > 0;
        self.mode1_vblank_check_enable = (addr & 0x10) > 0;
        self.mode0_hblank_check_enable = (addr & 0x08) > 0;
      }
      0xFF42 => { // Scroll x
        self.scroll_x = value;
      }
      0xFF43 => { // Scroll Y
        self.scroll_y = value;
      }
      0xFF45 => { // Scan line compare/LY Compare/LYC
        self.lyc_compare = value;
      }
      0xFF47 => { // Background palette
        // TODO: background palette
      }
      0xFF70 => { // wram bank
        self.wram_bank = value & 0x3;
      }
      0xFF4F => {
        self.vram_bank = value & 0x1;
      }
      _ => {
        // ignore
      }
    }
  }

  fn ioread(&self, addr: u16) -> u8 {
    match addr {
      0xFF40 => { // LCD/GPU control
        self.lcd_control.bits()
      }
      0xFF41 => { // LCD status/STAT
        let mut res = 0u8;
        if self.lyc_check_enable {
          res |= 1 << 6;
        }
        if self.mode2_oam_check_enable {
          res |= 1 << 5;
        }
        if self.mode1_vblank_check_enable {
          res |= 1 << 4;
        }
        if self.mode0_hblank_check_enable {
          res |= 1 << 3;
        }
        if self.lyc_compare == self.gpu_line {
          res |= 1 << 2;
        }
        if self.lcd_control.contains(LCDControlFlags::POWER) {
          res |= self.gpu_mode as u8;
        }
        res
      }
      0xFF42 => { // Scroll x
        self.scroll_x
      }
      0xFF43 => { // Scroll Y
        self.scroll_y
      }
      0xFF44 => { // Scan line/LY
        self.gpu_line
      }
      0xFF45 => { // Scan line compare/LY compare/LYC
        self.lyc_compare
      }
      0xFF47 => { // Background palette
        // TODO: background palette
        0xFF
      }
      0xFF70 => {
        0xF8 | self.wram_bank
      }
      0xFF4F => {
        0xFE | self.vram_bank
      }
      _ => {
        0xFF
      }
    }
  }
}
