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

class ROM {
public:
  static constexpr uint16_t BANK_SIZE = 0x4000;
  static constexpr uint16_t SRAM_SIZE = 0x4000;

private:
  std::vector<std::array<uint8_t, BANK_SIZE>> banks;
  std::array<uint8_t, SRAM_SIZE> sram{};

public:
  explicit ROM(std::vector<std::array<uint8_t, BANK_SIZE>> banks);

  uint8_t read(uint8_t bank, uint16_t offset);
  void write(uint8_t bank, uint16_t offset, uint8_t value);

  uint8_t readSram(uint16_t offset);
  void writeSram(uint16_t offset, uint8_t value);

  static std::shared_ptr<ROM> readRom(FILE *file);
};


#endif //PGB_ROM_HPP
