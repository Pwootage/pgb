#include "../../include/pgb/MMU.hpp"
#include "../../include/pgb/GPU.hpp"

#include <utility>

MMU::MMU(std::shared_ptr<ROM> rom) : rom(std::move(rom)) {}

uint8_t MMU::read8(uint64_t clock, uint16_t addr) {
  if (ROM0::addrIsBelow(addr)) {
    // ROM0
    if (!cartInserted) {
      return 0xFF;
    } else {
      return rom->read(0, addr);
    }
  } else if (ROMX::addrIsBelow(addr)) {
    // ROMX
    if (!cartInserted) {
      return 0xFF;
    } else {
      return rom->read(rom_bank, addr - ROMX::start);
    }
  } else if (VRAM::addrIsBelow(addr)) {
    // VRAM
    if (lcdPower() && gpu_mode == GPU_MODE::SCAN_VRAM) {
      return 0xFF;
    }
    return vram[addr - VRAM::start];
  } else if (SRAM::addrIsBelow(addr)) {
    // SRAM
    if (sramEnable) {
      return rom->readSram(addr - SRAM::start);
    } else {
      return 0xFF;
    }
  } else if (WRAM0::addrIsBelow(addr)) {
    // WRAM0
    return wramx[0][(addr - WRAM0::start) % WRAM0::size];
  } else if (WRAMX::addrIsBelow(addr)) {
    // WRAMAX
    auto bank = ((model > GBMode::GBC) ? wram_bank : 1) % wramx.size();
    if (bank == 0) bank = 1;
    return wramx[bank][(addr - WRAMX::start) % WRAMX::size];
  } else if (ECHO::addrIsBelow(addr)) {
    // Weird unused echo memory
    if (unusedMemoryDuplicateMode) {
      uint8_t wram = read8(clock, addr - ECHO::start + WRAM0::start);
      uint8_t sram = read8(clock, addr - ECHO::start + SRAM::start);
      return wram & sram;
    } else {
      return read8(clock, addr - ECHO::start + WRAM0::start);
    }
  } else if (OAM::addrIsBelow(addr)) {
    // Object attribute table (sprite info table)
    uint16_t off = addr - OAM::start;
    if (off >= oam.size()) return 0xFF;
    if (lcdPower() && (gpu_mode == GPU_MODE::SCAN_OAM || gpu_mode == GPU_MODE::SCAN_VRAM)) return 0xFF;
    return oam[off];
  } else if (UNUSED::addrIsBelow(addr)) {
    //TODO unused area weirdness depending on mode
    return 0;
  } else if (IO::addrIsBelow(addr)) {
    return ioread(addr);
  } else if (HRAM::addrIsBelow(addr)) {
    // Internal CPU ram
    return hram[addr - HRAM::start];
  } else /*if(IE::addrIsBelow(addr))*/ {
    // Interrupt enable flags
    return interrupt_enable;
  }
}

void MMU::write8(uint64_t clock, uint16_t addr, uint8_t value) {
  if (ROM0::addrIsBelow(addr)) {
    //ROM0
    if (cartInserted) {
      rom->write(0, addr, value);
    }
  } else if (ROMX::addrIsBelow(addr)) {
    //ROMX
    if (cartInserted) {
      rom->write(rom_bank, addr, value);
    }
  } else if (VRAM::addrIsBelow(addr)) {
    //VRAM
    if (!lcdPower() || gpu_mode != GPU_MODE::SCAN_VRAM) {
      vram[addr - VRAM::start] = value;
    }
  } else if (SRAM::addrIsBelow(addr)) {
    //SRAM
    if (sramEnable) {
      rom->writeSram(addr - SRAM::start, value);
    }
  } else if (WRAM0::addrIsBelow(addr)) {
    //WRAM0
    wramx[0][(addr - WRAM0::start) % WRAM0::size] = value;
  } else if (WRAMX::addrIsBelow(addr)) {
    //WRAMX
    auto bank = ((model > GBMode::GBC) ? wram_bank : 1) % wramx.size();
    wramx[bank][(addr - WRAMX::start)] = value;
  } else if (ECHO::addrIsBelow(addr)) {
    // Weird mirror unused memory
    if (unusedMemoryDuplicateMode) {
      write8(clock, addr - ECHO::start + WRAM0::start, value);
      write8(clock, addr - ECHO::start + SRAM::start, value);
    } else {
      write8(clock, addr - ECHO::start + WRAM0::start, value);
    }
  } else if (OAM::addrIsBelow(addr)) {
    uint16_t off = addr - OAM::start;
    if (off < oam.size()) {
      if (!lcdPower() || (gpu_mode != GPU_MODE::SCAN_OAM && gpu_mode != GPU_MODE::SCAN_VRAM)) {
        oam[off] = value;
      }
    }
  } else if (UNUSED::addrIsBelow(addr)) {
    //TODO unused area weirdness depending on mode
    // for now, ignore
  } else if (IO::addrIsBelow(addr)) {
    // IO registers
    iowrite(addr, value);
  } else if (HRAM::addrIsBelow(addr)) {
    // Internal CPU ram
    hram[addr - HRAM::start] = value;
  } else /*if(IE::addrIsBelow(addr))*/ {
    interrupt_enable = value;
  }
}

void MMU::iowrite(uint16_t addr, uint8_t value) {
  if (addr == 0xFF40) { // LCD/GPU control
    this->lcdControl = value;
  }
  if (addr == 0xFF42) { // Scroll x
    this->scrollX = value;
  }
  if (addr == 0xFF43) { // Scroll Y
    this->scrollY = value;
  }
  if (addr == 0xFF44) { // Scan line
    // ignore
  }
  if (addr == 0xFF47) { // Background palette
    // TODO: background palette
  }
  if (addr == 0xFF70) {
    this->wram_bank = value & 0x3u;
  }
  if (addr == 0xFF4F) {
    this->vram_bank = value & 0x1u;
  }
}

uint8_t MMU::ioread(uint16_t addr) const {
  if (addr == 0xFF40) { // LCD/GPU control
    return this->lcdControl;
  }
  if (addr == 0xFF42) { // Scroll x
    return this->scrollX;
  }
  if (addr == 0xFF43) { // Scroll Y
    return this->scrollY;
  }
  if (addr == 0xFF44) { // Scan line
    return gpu_line;
  }

  if (addr == 0xFF47) { // Background palette
    // TODO: background palette
  }
  if (addr == 0xFF70) {
    return 0xF8u | this->wram_bank;
  }
  if (addr == 0xFF4F) {
    return 0xFEu | this->vram_bank;
  }
  return 0xFF;
}

uint16_t MMU::read16(uint64_t clock, uint16_t addr) {
  return read8(clock, addr) | (read8(clock + 4, addr + 1) << 8u);
}

void MMU::write16(uint64_t clock, uint16_t addr, uint16_t value) {
  write8(clock, addr, value & 0xFFu);
  write8(clock + 4, addr, (value >> 8u) & 0xFFu);
}

uint8_t MMU::oamread(uint16_t addr) {
  if (addr < oam.size()) return oam[addr];
  return 0xFF;
}

uint8_t MMU::vramread(uint16_t addr) {
  if (addr < vram.size()) return vram[addr];
  return 0xFF;
}

uint8_t MMU::vram2read(uint16_t addr) {
  if (addr < vram2.size()) return vram2[addr];
  return 0xFF;
}