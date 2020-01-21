#ifndef PGB_ROM_HPP
#define PGB_ROM_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <array>

struct GBHeader {
  uint8_t vectors[0x100];
  uint8_t start[4];
  uint8_t logo[48];
  uint8_t gameTitle[11];
  uint8_t mfgCode[4];
  uint8_t gbcFlag;
  uint8_t newLicenseeCode[2];
  uint8_t sgbFlag;
  uint8_t cartType;
  uint8_t romSize;
  uint8_t ramSize;
  uint8_t destCode;
  uint8_t oldLicenseeCode;
  uint8_t romVersion;
  uint8_t headerChecksum;
  uint16_t globalChecksum;
};

enum class CART_TYPE {
  ROM_ONLY = 0x00,
  MBC1 = 0x01,
  MBC1_RAM = 0x02,
  MBC1_RAM_BATT = 0x03,
  MBC2 = 0x05,
  MBC2_RAM_BATT = 0x06,
  ROM_RAM = 0x08,
  ROM_RAM_BATT = 0x09,
  MMM01 = 0x0B,
  MMM01_RAM = 0x0C,
  MMM01_RAM_BATT = 0x0D,
  MBC3_TIMER_BATT = 0x0F,
  MBC3_RAM_TIMER_BATT = 0x10,
  MBC3 = 0x11,
  MBC3_RAM = 0x12,
  MBC3_RAM_BATTERY = 0x13,
  MBC5 = 0x19,
  MBC5_RAM = 0x1A,
  MBC5_RAM_BATTERY = 0x1B,
  MBC5_RUMBLE = 0x1C,
  MBC5_RAM_RUMBLE = 0x1D,
  MBC5_RAM_BATTERY_RUMBLE = 0x1E,
  MBC6_RAM_BAT = 0x20,
  MBC7_RAM_BAT_ACCEL = 0x22,
  POCKET_CAM = 0xFC,
  BANDAI_TAMA5 = 0xFD,
  HuC3 = 0xFE,
  HuC1_RAM_BAT = 0xFF
};

class ROM {
public:
  static constexpr uint16_t BANK_SIZE = 0x4000;
  static constexpr uint16_t SRAM_SIZE = 0x4000;
  bool ram_enabled = false;
  bool ram_mode = false;
  uint16_t rom_bank = 0x1;
  uint8_t ram_bank = 0x0;

private:
  std::vector<std::array<uint8_t, BANK_SIZE>> banks;
  std::array<std::array<uint8_t, SRAM_SIZE>, 4> sram{};
  CART_TYPE cartType;

public:
  explicit ROM(std::vector<std::array<uint8_t, BANK_SIZE>> banks);

  uint8_t read(uint16_t offset);
  void write(uint16_t offset, uint8_t value);

  uint8_t readSram(uint16_t offset);
  void writeSram(uint16_t offset, uint8_t value);

  GBHeader *header();

  static std::shared_ptr<ROM> readRom(FILE *file);
  static std::shared_ptr<ROM> readRom(std::vector<uint8_t>);
};


#endif //PGB_ROM_HPP
