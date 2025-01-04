
pub struct GBHeader {
  vectors: [u8; 0x100],
  start: [u8; 4],
  logo: [u8; 48],
  game_title: [u8; 11],
  mfg_code: [u8; 4],
  gbc_flag: u8,
  new_licensee_code: [u8; 2],
  sgb_flag: u8,
  cart_type: u8,
  rom_size: u8,
  ram_size: u8,
  dest_code: u8,
  old_licensee_code: u8,
  rom_version: u8,
  header_checksum: u8,
  global_checksum: u16,
}

#[derive(Debug, PartialEq, Copy, Clone)]
#[repr(u8)]
pub enum CartType {
  RomOnly = 0x00,
  MBC1 = 0x01,
  MBC1Ram = 0x02,
  MBC1RamBatt = 0x03,
  MBC2 = 0x05,
  MBC2RamBatt = 0x06,
  RomRam = 0x08,
  RomRamBatt = 0x09,
  MMM01 = 0x0B,
  MMM01Ram = 0x0C,
  MMM01RamBatt = 0x0D,
  MBC3TimerBatt = 0x0F,
  MBC3RamTimerBatt = 0x10,
  MBC3 = 0x11,
  MBC3Ram = 0x12,
  MBC3RamBattery = 0x13,
  MBC5 = 0x19,
  MBC5Ram = 0x1A,
  MBC5RamBattery = 0x1B,
  MBC5Rumble = 0x1C,
  MBC5RamRumble = 0x1D,
  MBC5RamBatteryRumble = 0x1E,
  MBC6RamBat = 0x20,
  MBC7RamBatAccel = 0x22,
  PocketCam = 0xFC,
  BandaiTama5 = 0xFD,
  HuC3 = 0xFE,
  HuC1RamBat = 0xFF,
}
impl TryFrom<u8> for CartType {
  type Error = ();

  fn try_from(value: u8) -> Result<Self, Self::Error> {
    // TODO: less dumb way of doing this... probably an enum crate
    match value {
      0x00 => Ok(CartType::RomOnly),
      0x01 => Ok(CartType::MBC1),
      0x02 => Ok(CartType::MBC1Ram),
      0x03 => Ok(CartType::MBC1RamBatt),
      0x05 => Ok(CartType::MBC2),
      0x06 => Ok(CartType::MBC2RamBatt),
      0x08 => Ok(CartType::RomRam),
      0x09 => Ok(CartType::RomRamBatt),
      0x0B => Ok(CartType::MMM01),
      0x0C => Ok(CartType::MMM01Ram),
      0x0D => Ok(CartType::MMM01RamBatt),
      0x0F => Ok(CartType::MBC3TimerBatt),
      0x10 => Ok(CartType::MBC3RamTimerBatt),
      0x11 => Ok(CartType::MBC3),
      0x12 => Ok(CartType::MBC3Ram),
      0x13 => Ok(CartType::MBC3RamBattery),
      0x19 => Ok(CartType::MBC5),
      0x1A => Ok(CartType::MBC5Ram),
      0x1B => Ok(CartType::MBC5RamBattery),
      0x1C => Ok(CartType::MBC5Rumble),
      0x1D => Ok(CartType::MBC5RamRumble),
      0x1E => Ok(CartType::MBC5RamBatteryRumble),
      0x20 => Ok(CartType::MBC6RamBat),
      0x22 => Ok(CartType::MBC7RamBatAccel),
      0xFC => Ok(CartType::PocketCam),
      0xFD => Ok(CartType::BandaiTama5),
      0xFE => Ok(CartType::HuC3),
      0xFF => Ok(CartType::HuC1RamBat),
      _ => Err(()),
    }
  }
}


const BANK_SIZE: usize = 0x4000;
const SRAM_SIZE: usize = 0x4000;

pub struct ROM {
  pub sram_enabled: bool,
  pub sram_mode: bool,
  pub rom_bank: u16,
  pub sram_bank: u8,

  banks: Vec<[u8; BANK_SIZE]>,
  sram: [[u8; SRAM_SIZE]; 4],

  pub cart_type: CartType,
}

impl ROM {
  pub fn new(bytes: Vec<u8>) -> Self {
    let mut banks: Vec<[u8; BANK_SIZE]> = Vec::new();
    for i in( 0..bytes.len()).step_by(BANK_SIZE) {
      let mut read = bytes.len() - i;
      if read > BANK_SIZE {
        read = BANK_SIZE;
      }
      let mut bank = [0; BANK_SIZE];
      for j in 0..read {
        bank[j] = bytes[i + j];
      }
      banks.push(bank);
    }

    let mut rom = ROM {
      sram_enabled: false,
      sram_mode: false,
      rom_bank: 1,
      sram_bank: 0,
      banks,
      sram: [[0; SRAM_SIZE]; 4], // TODO: persist this
      cart_type: CartType::RomOnly,
    };

    let header = rom.header();
    rom.cart_type = CartType::try_from(header.cart_type).unwrap(); // todo: error handling
    rom
  }

  pub fn new_from_file(file: &str) -> Self {
    let bytes = std::fs::read(file).unwrap();
    ROM::new(bytes)
  }
}

impl ROM {
  pub fn header(&self) -> GBHeader {
    // TODO: an auto parser? I was using reinterpret_cast in C++ to do this (which is safe)
    let mut header = GBHeader {
      vectors: [0; 0x100],
      start: [0; 4],
      logo: [0; 48],
      game_title: [0; 11],
      mfg_code: [0; 4],
      gbc_flag: 0,
      new_licensee_code: [0; 2],
      sgb_flag: 0,
      cart_type: 0,
      rom_size: 0,
      ram_size: 0,
      dest_code: 0,
      old_licensee_code: 0,
      rom_version: 0,
      header_checksum: 0,
      global_checksum: 0,
    };

    for i in 0..0x100 {
      header.vectors[i] = self.banks[0][i];
    }

    for i in 0..4 {
      header.start[i] = self.banks[0][0x100 + i];
    }

    for i in 0..48 {
      header.logo[i] = self.banks[0][0x104 + i];
    }

    for i in 0..11 {
      header.game_title[i] = self.banks[0][0x134 + i];
    }

    for i in 0..4 {
      header.mfg_code[i] = self.banks[0][0x13F + i];
    }

    header.gbc_flag = self.banks[0][0x143];
    for i in 0..2 {
      header.new_licensee_code[i] = self.banks[0][0x144 + i];
    }

    header.sgb_flag = self.banks[0][0x146];
    header.cart_type = self.banks[0][0x147];
    header.rom_size = self.banks[0][0x148];
    header.ram_size = self.banks[0][0x149];
    header.dest_code = self.banks[0][0x14A];
    header.old_licensee_code = self.banks[0][0x14B];
    header.rom_version = self.banks[0][0x14C];
    header.header_checksum = self.banks[0][0x14D];
    header.global_checksum = (self.banks[0][0x14E] as u16) | ((self.banks[0][0x14F] as u16) << 8);

    header
  }

  pub fn read(&self, offset: u16) -> u8 {
    if offset < BANK_SIZE as u16 {
      self.banks[0][offset as usize]
    } else {
      self.banks[self.rom_bank as usize % self.banks.len()][offset as usize - BANK_SIZE]
    }
  }

  pub fn write(&mut self, offset: u16, value: u8) {
    match self.cart_type {
      // MBC1
      CartType::MBC1 | CartType::MBC1Ram | CartType::MBC1RamBatt => {
        if offset < 0x1FFF {
          self.sram_enabled = (value & 0xF) == 0xA;
        } else if offset < 0x3FFF {
          let mut new_bank = (value & 0x1F) as u16;
          if new_bank == 0 {
            new_bank = 1;
          }
          self.rom_bank = (self.rom_bank & 0xE0) | new_bank;
        } else if offset < 0x5FFF {
          let bank = value & 0x3;
          if self.sram_mode {
            self.sram_bank = bank;
          } else {
            self.rom_bank = (self.rom_bank & 0x1F) | (bank << 5) as u16
          }
        } else {
          let flag = (value & 1) != 0;
          if flag {
            self.sram_mode = true;
            self.rom_bank = self.rom_bank & 0x1F;
          } else {
            self.sram_mode = false;
          }
        }
      }
      // MBC2
      CartType::MBC2 | CartType::MBC2RamBatt => {
        if offset < 0x1FFF {
          if (offset & 0x0100) == 0 {
            self.sram_enabled = (value & 0xF) == 0xA;
          }
        } else if offset < 0x3FFF {
          if (offset & 0x0100) != 0 {
            let mut new_bank = (value & 0x1F) as u16;
            if new_bank == 0 {
              new_bank = 1;
            }
            self.rom_bank = (self.rom_bank & 0xE0) | new_bank;
          }
        }
      }
      // MBC3
      CartType::MBC3
      | CartType::MBC3Ram
      | CartType::MBC3RamBattery
      | CartType::MBC3TimerBatt
      | CartType::MBC3RamTimerBatt => {
        if offset < 0x1FFF {
          self.sram_enabled = (value & 0xF) == 0xA;
        } else if offset < 0x3FFF {
          let mut new_bank = (value & 0x7F) as u16;
          if new_bank == 0 {
            new_bank = 1;
          }
          self.rom_bank = new_bank;
        } else if offset < 0x5FFF {
          // TODO: ram bank/rgc register
        } else {
          // TODO clock
        }
      }
      // mbc5
      CartType::MBC5
      | CartType::MBC5Ram
      | CartType::MBC5RamBattery
      | CartType::MBC5RamRumble
      | CartType::MBC5RamBatteryRumble => {
        if offset < 0x1FFF {
          self.sram_enabled = (value & 0xF) == 0xA;
        } else if offset < 0x2FFF {
          let mut new_bank = value as u16;
          if new_bank == 0 {
            new_bank = 1;
          }
          self.rom_bank = (self.rom_bank & 0xFF00) | new_bank;
        } else if offset < 0x3FFF {
          let mut new_bank = value as u16;
          if self.banks.len() < 256 {
            if new_bank == 0 {
              new_bank = 1;
            }
            self.rom_bank = (self.rom_bank & 0xFF00) | new_bank;
          } else {
            self.rom_bank = (self.rom_bank & 0xFF) | (new_bank << 8);
          }
        } else {
          self.sram_bank = value & 0x3;
          // TODO: rumble, different ram sizes
        }
      }
      CartType::RomOnly | CartType::RomRam | CartType::RomRamBatt => {
        // ignore
      }
      // unsupported
      _ => {
        // ignore
      }
    }
  }

  pub fn read_sram(&self, offset: u16) -> u8 {
    if self.sram_enabled {
      let bank = if self.sram_mode {
        0
      } else {
        self.sram_bank as usize % self.sram.len()
      };
      self.sram[bank][offset as usize % self.sram[bank].len()]
    } else {
      0xFF
    }
  }

  pub fn write_sram(&mut self, offset: u16, value: u8) {
    if self.sram_enabled {
      let bank = if self.sram_mode {
        0
      } else {
        self.sram_bank as usize % self.sram.len()
      };
      self.sram[bank][offset as usize % self.sram[bank].len()] = value;
    }
  }
}
