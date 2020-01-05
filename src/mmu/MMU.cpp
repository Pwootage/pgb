#include "MMU.hpp"

#include <utility>

MMU::MMU(std::shared_ptr<ROM> rom) : rom(std::move(rom)) {}

uint8_t MMU::read8(uint16_t addr) {
  if (ROM0::addrIsBelow(addr)) {

  } else if (ROMX::addrIsBelow(addr)) {

  } else if (VRAM::addrIsBelow(addr)) {

  } else if (SRAM::addrIsBelow(addr)) {

  } else if (WRAM0::addrIsBelow(addr)) {

  } else if (WRAMX::addrIsBelow(addr)) {

  } else if (ECHO::addrIsBelow(addr)) {

  } else if (OAM::addrIsBelow(addr)) {

  } else if (UNUSED::addrIsBelow(addr)) {

  } else if (IO::addrIsBelow(addr)) {

  } else if (HRAM::addrIsBelow(addr)) {

  } else /*if(IE::addrIsBelow(addr))*/ {

  }
}
