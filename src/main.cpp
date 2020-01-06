#include <iostream>
#include "rom/ROM.hpp"
#include "mmu/MMU.hpp"
#include "cpu/CPU.hpp"

int main() {
  FILE *romFile = fopen("testRoms/basic_tests/build/test.gb", "rb");
  std::shared_ptr<ROM> rom = ROM::readRom(romFile);
  fclose(romFile);
  std::shared_ptr<MMU> mmu(new MMU(rom));

  CPU cpu(mmu);

  cpu.printState();
  for (int i = 0; i < 3; i++) {
    cpu.emulateInstruction();
    cpu.printState();
  }

  return 0;
}
