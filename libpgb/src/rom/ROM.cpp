#include "../../include/pgb/ROM.hpp"

#include <utility>
#include <cmath>

ROM::ROM(std::vector<std::array<uint8_t, BANK_SIZE>> banks) : banks(std::move(banks)) {
  cartType = static_cast<CART_TYPE>(this->header()->cartType);
}

uint8_t ROM::read(uint16_t offset) {
  if (offset < BANK_SIZE) {
    return banks[0][offset];
  }
  return banks[rom_bank % banks.size()][offset - BANK_SIZE];
}

void ROM::write(uint16_t offset, uint8_t value) {
  switch (cartType) {
    case CART_TYPE::MBC1:
    case CART_TYPE::MBC1_RAM:
    case CART_TYPE::MBC1_RAM_BATT:
      if (offset < 0x1FFF) {
        ram_enabled = (value & 0xF) == 0xA;
      } else if (offset < 0x3FFF) {
        uint8_t newBank = value & 0x1F;
        if (newBank == 0) newBank = 1;
        rom_bank = (rom_bank & 0xE0) | newBank;
      } else if (offset < 0x5FFF) {
        uint8_t bank = value & 0x3;
        if (ram_mode) {
          ram_bank = bank;
        } else {
          rom_bank = (rom_bank & 0x1F) | (bank << 5);
        }
      } else {
        bool flag = (value & 1) != 0;
        if (flag) {
          ram_mode = true;
          rom_bank = rom_bank & 0x1F;
        } else {
          ram_mode = false;
        }
      }
      break;
    case CART_TYPE::MBC2:
    case CART_TYPE::MBC2_RAM_BATT:
      if (offset < 0x1FFF) {
        if ((offset & 0x0100) == 0) {
          ram_enabled = (value & 0xF) == 0xA;
        }
      } else if (offset < 0x3FFF) {
        if ((offset & 0x0100) != 0) {
          uint8_t newBank = value & 0x1F;
          if (newBank == 0) newBank = 1;
          rom_bank = (rom_bank & 0xE0) | newBank;
        }
      }
      break;
    case CART_TYPE::MBC3_TIMER_BATT:
    case CART_TYPE::MBC3_RAM_TIMER_BATT:
    case CART_TYPE::MBC3:
    case CART_TYPE::MBC3_RAM:
    case CART_TYPE::MBC3_RAM_BATTERY:
      if (offset < 0x1FFF) {
        ram_enabled = (value & 0xF) == 0xA;
      } else if (offset < 0x3FFF) {
        uint8_t newBank = value & 0x7F;
        if (newBank == 0) newBank = 1;
        rom_bank = newBank;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCSimplifyInspection"
      } else if (offset < 0x5FFF) {
        // TODO: ram bank/rgc register
      } else {
        // TODO clock
      }
#pragma clang diagnostic pop
      break;
    case CART_TYPE::MBC5:
    case CART_TYPE::MBC5_RAM:
    case CART_TYPE::MBC5_RAM_BATTERY:
    case CART_TYPE::MBC5_RUMBLE:
    case CART_TYPE::MBC5_RAM_RUMBLE:
    case CART_TYPE::MBC5_RAM_BATTERY_RUMBLE:
      if (offset < 0x1FFF) {
        ram_enabled = (value & 0xF) == 0xA;
      } else if (offset < 0x2FFF) {
        uint8_t newBank = value;
        if (newBank == 0) newBank = 1;
        rom_bank = (rom_bank & 0xFF00) | newBank;
      } else if (offset < 0x3FFF) {
        uint8_t newBank = value;
        if (banks.size() < 256) {
          if (newBank == 0) newBank = 1;
          rom_bank = (rom_bank & 0xFF00) | newBank;
        } else {
          rom_bank = (rom_bank & 0xFF) | (newBank << 8);
        }
      } else {
        ram_bank = value & 0x3;
        // TODO: rumble, different ram sizes
      }
      break;

      // unsupported cart types
    case CART_TYPE::MBC6_RAM_BAT:
    case CART_TYPE::MBC7_RAM_BAT_ACCEL:
    case CART_TYPE::MMM01:
    case CART_TYPE::MMM01_RAM:
    case CART_TYPE::MMM01_RAM_BATT:
    case CART_TYPE::POCKET_CAM:
    case CART_TYPE::BANDAI_TAMA5:
    case CART_TYPE::HuC3:
    case CART_TYPE::HuC1_RAM_BAT:
      // rom-only carts ignore
    case CART_TYPE::ROM_ONLY:
    case CART_TYPE::ROM_RAM:
    case CART_TYPE::ROM_RAM_BATT:
    default:
      // Ignore
      break;
  }
}

uint8_t ROM::readSram(uint16_t offset) {
  if (ram_enabled) {
    return sram[ram_mode ? 0 : (ram_bank % 4)][offset];
  } else {
    return 0xFF;
  }
}

void ROM::writeSram(uint16_t offset, uint8_t value) {
  if (ram_enabled) {
    sram[ram_mode ? 0 : (ram_bank % 4)][offset] = value;
  }
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

std::shared_ptr<ROM> ROM::readRom(std::vector<uint8_t> bytes) {
  std::vector<std::array<uint8_t, BANK_SIZE>> banks;
  for (int i = 0; i < bytes.size(); i += BANK_SIZE) {
    banks.emplace_back();
    size_t read = bytes.size() - i;
    if (read > BANK_SIZE) read = BANK_SIZE;
    memcpy(banks[banks.size() - 1].data(), bytes.data() + i, read);
  }
  return std::shared_ptr<ROM>(new ROM(std::move(banks)));
}

GBHeader *ROM::header() {
  return reinterpret_cast<GBHeader *>(&(banks[0][0x100]));
}
