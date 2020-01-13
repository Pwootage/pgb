#include "../../include/pgb/ROM.hpp"

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

std::shared_ptr<ROM> ROM::readRom(FILE *file) {
  // TODO: safety checks lmao
  std::vector<std::array<uint8_t, BANK_SIZE>> banks;

  int read = 0;
  std::array<uint8_t, BANK_SIZE> buffer{};
  while ((read = fread(buffer.data(), 1, BANK_SIZE, file)) > 0) {
    banks.emplace_back();
    memcpy(banks[banks.size() - 1].data(), buffer.data(), read);
  }

  return std::shared_ptr<ROM>(new ROM(std::move(banks)));
}
