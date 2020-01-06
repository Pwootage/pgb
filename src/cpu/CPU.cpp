#include <iostream>
#include "CPU.hpp"
#include "interpreter/interpreter.hpp"

constexpr uint16_t GB_INIT[]{
  0x01B0u,
  0x0013u,
  0x00D8u,
  0x014Du
};

constexpr uint16_t GBP_INIT[]{
  0xffb0u,
  0x0013u,
  0x00D8u,
  0x014Du
};

constexpr uint16_t GBC_INIT[]{
  0x1180u,
  0x0000u,
  0x0008u,
  0x007cu
};

constexpr uint16_t SGB_INIT[]{
  0x0100u,
  0x0014u,
  0x0000u,
  0xC060u
};

constexpr uint16_t SGB2_INIT[]{
  0xFF00u,
  0x0000u,
  0x0000u,
  0x0000u
};

constexpr uint16_t GBA_INIT[]{
  0x1100u,
  0x0100u,
  0x0008u,
  0x007Cu
};

constexpr uint16_t GBC_INIT_GBC[]{
  0x1180u,
  0x0000u,
  0xFF56u,
  0x000Du
};

constexpr uint16_t GBA_INIT_GBC[]{
  0x1100u,
  0x0100u,
  0xFF56u,
  0x000Du
};


void CPU::initializeRegisters() {
  sp(0xFFFEu);
  pc(0x0100u);
  switch (mmu->model) {
    case GBMode::GB:
      af(GB_INIT[0]);
      bc(GB_INIT[1]);
      de(GB_INIT[2]);
      hl(GB_INIT[3]);
      break;
    case GBMode::GBP:
      af(GBP_INIT[0]);
      bc(GBP_INIT[1]);
      de(GBP_INIT[2]);
      hl(GBP_INIT[3]);
      break;
    case GBMode::SGB:
      af(SGB_INIT[0]);
      bc(SGB_INIT[1]);
      de(SGB_INIT[2]);
      hl(SGB_INIT[3]);
      break;
    case GBMode::SGB2:
      af(SGB2_INIT[0]);
      bc(SGB2_INIT[1]);
      de(SGB2_INIT[2]);
      hl(SGB2_INIT[3]);
      break;
    case GBMode::GBC:
      if (mmu->gbcMode) {
        af(GBC_INIT_GBC[0]);
        bc(GBC_INIT_GBC[1]);
        de(GBC_INIT_GBC[2]);
        hl(GBC_INIT_GBC[3]);
      } else {
        af(GBC_INIT[0]);
        bc(GBC_INIT[1]);
        de(GBC_INIT[2]);
        hl(GBC_INIT[3]);
      }
      break;
    case GBMode::GBA:
    case GBMode::GBASP:
      if (mmu->gbcMode) {
        af(GBA_INIT_GBC[0]);
        bc(GBA_INIT_GBC[1]);
        de(GBA_INIT_GBC[2]);
        hl(GBA_INIT_GBC[3]);
      } else {
        af(GBA_INIT[0]);
        bc(GBA_INIT[1]);
        de(GBA_INIT[2]);
        hl(GBA_INIT[3]);
      }
      break;
  }
}

CPU::CPU(std::shared_ptr<MMU> mmu) : mmu(std::move(mmu)) {
  initializeRegisters();
}

void CPU::emulateInstruction() {
  uint8_t op = pcRead8();
  // Don't PCMod, this has already consumed a cycle
  pc(pc() + 1);

  interpreter::ops[op](this);
}

void CPU::printState() {
  printf("====%.08d====\n", clock_t);
  printf("AF %0.4x ", af());
  printf(" BC: %0.4x ", bc());
  if (zero()) printf("Z");
  if (addSub()) printf("N");
  if (halfCarry()) printf("H");
  if (carry()) printf("C");
  printf("\nDE: %0.4x ", de());
  printf("HL: %0.4x\n", hl());
  printf("SP: %0.4x ", sp());
  printf("PC: %0.4x\n", pc());
  fflush(stdout);
}
