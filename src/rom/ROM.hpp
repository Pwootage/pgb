#ifndef PGB_ROM_HPP
#define PGB_ROM_HPP

#include <cstdint>
#include <memory>
#include <vector>

class ROM {
public:
  static constexpr uint16_t BANK_SIZE = 0x4000;
  static constexpr uint16_t SRAM_SIZE = 0x4000;

private:
  std::vector<uint8_t[BANK_SIZE]> banks;
  uint8_t sram[SRAM_SIZE];

public:
  explicit ROM(std::vector<uint8_t[BANK_SIZE]> banks);


  void switchBank(uint8_t bank);

  uint8_t read(uint16_t offset);
  void write(uint16_t offset, uint8_t value);

  uint8_t readSram(uint16_t offset);
  void writeSram(uint16_t offset, uint8_t value);
};


#endif //PGB_ROM_HPP
