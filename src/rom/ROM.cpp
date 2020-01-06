#include "ROM.hpp"

#include <utility>

ROM::ROM(std::vector<std::array<uint8_t, BANK_SIZE>> banks) : banks(std::move(banks)) {}

uint8_t ROM::read(uint8_t bank, uint16_t offset) {
  return banks[bank % banks.size()][offset];
}

void ROM::write(uint8_t bank, uint16_t offset, uint8_t value) {
  // Ignore for default rom
}

uint8_t ROM::readSram(uint16_t offset) {
  return sram[offset];
}

void ROM::writeSram(uint16_t offset, uint8_t value) {
  sram[offset] = value;
}


