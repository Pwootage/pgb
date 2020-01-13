#ifndef PGB_MMU_HPP
#define PGB_MMU_HPP

#include <cstdint>
#include <memory>
#include <array>
#include "../rom/ROM.hpp"
#include "../gb_mode.hpp"

// start and end are inclusive
template<uint16_t startAddr, uint16_t endAddr>
struct MemoryMap {
  static_assert(startAddr < endAddr, "End must be after start");
  static constexpr uint16_t start = startAddr;
  static constexpr uint16_t end = endAddr;
  static constexpr uint16_t size = endAddr - startAddr;

  inline static constexpr bool inRange(uint16_t addr) {
    return addr >= start && addr <= end;
  }

  inline static constexpr bool addrIsBelow(uint16_t addr) {
    return addr <= end;
  }
};

enum class GPU_MODE {
  SCAN_OAM = 2,
  SCAN_VRAM = 3,
  HBLANK = 0,
  VBLANK = 1
};

enum class BACKGROUND_MODE {
  BG,
  WINDOW
};

class MMU {
public:
  using ROM0 = MemoryMap<0x0000, 0x3FFF>;
  using ROMX = MemoryMap<0x4000, 0x7FFF>;
  using VRAM = MemoryMap<0x8000, 0x9FFF>;
  using SRAM = MemoryMap<0xA000, 0xBFFF>;
  using WRAM0 = MemoryMap<0xC000, 0xCFFF>;
  using WRAMX = MemoryMap<0xD000, 0xDFFF>;
  using ECHO = MemoryMap<0xE000, 0xFDFF>;
  using OAM = MemoryMap<0xFE00, 0xFE9F>;
  using UNUSED = MemoryMap<0xFEA0, 0xFEFF>;
  using IO = MemoryMap<0xFF00, 0xFF7F>;
  using HRAM = MemoryMap<0xFF80, 0xFFFE>;
  using IE = MemoryMap<0xFFFF, 0xFFFF>;


public:
  explicit MMU(std::shared_ptr<ROM> rom);

  std::shared_ptr<ROM> rom;

  uint8_t read8(uint64_t clock, uint16_t addr);
  void write8(uint64_t clock, uint16_t addr, uint8_t value);

  uint8_t oamread(uint16_t addr);
  uint8_t vramread(uint16_t addr);
  uint8_t vram2read(uint16_t addr);

  uint16_t read16(uint64_t clock, uint16_t addr);
  void write16(uint64_t clock, uint16_t addr, uint16_t value);

  uint8_t wram_bank{1};
  uint8_t vram_bank{0};
  uint8_t rom_bank{1};
  uint8_t interrupt_enable = 0;
  bool interrupts_enabled = false;
  GPU_MODE gpu_mode = GPU_MODE::SCAN_OAM;
  uint64_t gpu_line = 0;
  GBMode model = GBMode::GB;
  bool gbcMode = false;
  bool unusedMemoryDuplicateMode = false;
  bool sramEnable = true;
  bool cartInserted = true;
  uint8_t scrollX;
  uint8_t scrollY;
  uint8_t lcdControl = 0x80u;

  inline bool interrupt_joypad() { return (interrupt_enable & 0x10u) != 0; }
  inline bool interrupt_serial() { return (interrupt_enable & 0x8u) != 0; }
  inline bool interrupt_timer() { return (interrupt_enable & 0x4u) != 0; }
  inline bool interrupt_lcd_stat() { return (interrupt_enable & 0x2u) != 0; }
  inline bool interrupt_vblank() { return (interrupt_enable & 0x1u) != 0; }

  inline bool lcdPower() { return (lcdControl & 0x80) != 0; }
  inline bool lcdWindowTiles() { return (lcdControl & 0x40) != 0; }
  inline bool lcdWindowEnable() { return (lcdControl & 0x20) != 0; }
  inline uint8_t lcdBGWindowTileset() { return (lcdControl & 0x10) != 0 ? 1 : 0; }
  inline uint8_t lcdBGTileMap() { return (lcdControl & 0x8) != 0 ? 1 : 0; }
  inline uint8_t lcdSpriteSize() { return (lcdControl & 0x4) != 0 ? 1 : 0; }
  inline bool lcdSpritesEnabled() { return (lcdControl & 0x2) != 0; }
  inline bool bgEnabled() { return (lcdControl & 0x1) != 0; }


private:
  std::array<uint8_t, VRAM::size> vram{};
  std::array<uint8_t, VRAM::size> vram2{};
  std::array<std::array<uint8_t, VRAM::size>, 8> wramx{};
  std::array<uint8_t, HRAM::size> hram{};
  std::array<uint8_t, 160> oam{};
  void iowrite(uint16_t addr, uint8_t value);
  uint8_t ioread(uint16_t addr) const;
};


#endif //PGB_MMU_HPP
