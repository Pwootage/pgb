#include <iostream>
#include "rom/ROM.hpp"
#include "mmu/MMU.hpp"
#include "cpu/CPU.hpp"
#include "gpu/GPU.hpp"

int main() {
  FILE *romFile = fopen("testRoms/basic_tests/build/test.gb", "rb");
  std::shared_ptr<ROM> rom = ROM::readRom(romFile);
  fclose(romFile);
  std::shared_ptr<MMU> mmu(new MMU(rom));

  CPU cpu(mmu);
  GPU gpu(mmu);

  cpu.printState();
  for (int i = 0; i < 3; i++) {
    uint64_t startClock = cpu.clock();
    cpu.emulateInstruction();
    uint64_t endClock = cpu.clock();
    gpu.update(endClock - startClock);
    cpu.printState();
  }

  return 0;
}
