#include <iostream>
#include "pgb/ROM.hpp"
#include "pgb/MMU.hpp"
#include "pgb/CPU.hpp"
#include "pgb/GPU.hpp"
#include <SDL.h>

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Failed to init sdl: " << SDL_GetError() << std::endl;
    return 1;
  }
  SDL_Window *window = SDL_CreateWindow(
    "PGP",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    512, 512,
    SDL_WINDOW_SHOWN
  );
  if (window == nullptr) {
    std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
    return 1;
  }

//  SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
//  SDL_UpdateWindowSurface(window);
  SDL_Renderer *renderer = SDL_CreateRenderer(
    window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
  );
  SDL_Texture *gbTex = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_STREAMING,
    LINE_WIDTH, LINES
  );

//  FILE *romFile = fopen("testRoms/basic_tests/build/test.gb", "rb");
//  FILE *romFile = fopen("testRoms/other/opus5.gb", "rb");
//  FILE *romFile = fopen("testRoms/blargg/cpu_instrs/individual/09-op r,r.gb", "rb");
  FILE *romFile = fopen("testRoms/other/ttt.gb", "rb");
  std::shared_ptr<ROM> rom = ROM::readRom(romFile);
  fclose(romFile);
  std::shared_ptr<MMU> mmu(new MMU(rom));

  CPU cpu(mmu);
  GPU gpu(mmu);

  cpu.printState();
  bool quit = false;
  uint64_t frame = 0;
  while (!quit) {
    uint64_t startClock = cpu.clock();
    cpu.emulateInstruction();
    uint64_t endClock = cpu.clock();
    gpu.update(endClock - startClock);
    uint64_t newFrame = endClock / CLOCK_FRAME;
    if (newFrame != frame) {
      frame = newFrame;
      cpu.printState();
      // TODO: copy buffer over
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          quit = true;
        }
        // Ignore other events
      }

      SDL_UpdateTexture(
        gbTex,
        nullptr,
        gpu.vsyncBuffer.data(), LINE_WIDTH * 2
      );

      SDL_RenderClear(renderer);
      SDL_RenderCopy(
        renderer,
        gbTex, nullptr, nullptr
      );
      SDL_RenderPresent(renderer);
      SDL_Delay(16);
    }
  }

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
