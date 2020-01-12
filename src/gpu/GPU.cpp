#include "GPU.hpp"

#include <utility>

GPU::GPU(std::shared_ptr<MMU> mmu) : mmu(std::move(mmu)) {}

void GPU::update(uint64_t clockDelta) {
  // TODO: there might be a cleaner solution
  gpuClock += clockDelta;
  switch (mmu->gpu_mode) {
    case GPU_MODE::SCAN_OAM:
      if (gpuClock > SCANLINE_OAM) {
        gpuClock -= SCANLINE_OAM;
        mmu->gpu_mode = GPU_MODE::SCAN_OAM;
      }
      break;
    case GPU_MODE::SCAN_VRAM:
      if (gpuClock > SCANLINE_VRAM) {
        gpuClock -= SCANLINE_VRAM;
        mmu->gpu_mode = GPU_MODE::HBLANK;
        renderLine();
      }
      break;
    case GPU_MODE::HBLANK:
      if (gpuClock > SCANLINE_HBLANK) {
        gpuClock -= SCANLINE_HBLANK;
        line++;
        if (line == LINES + 1) {
          writeToVsyncBuffer();
          mmu->gpu_mode = GPU_MODE::VBLANK;
        } else {
          mmu->gpu_mode = GPU_MODE::SCAN_OAM;
        }
      }
      break;
    case GPU_MODE::VBLANK:
      if (gpuClock > SCANLINE) {
        gpuClock -= SCANLINE;
        line++;
        if (line > LINES + 10) {
          mmu->gpu_mode = GPU_MODE::SCAN_OAM;
          line = 0;
        }
      }
      break;
  }
}

void GPU::renderLine() {
  uint16_t lineStartOffset = mmu->lcdBGTileMap() == 0 ? 0x1800 : 0x1c00;
  uint8_t tileset = mmu->lcdBGWindowTileset();
  uint16_t tilesetOffset = tileset == 0 ? 0x800 : 0x0;

  // add the offset to the beginning of the line where we care
  lineStartOffset += ((line + mmu->scrollY) & 0xFF) >> 3;
  uint8_t lineTile = mmu->scrollX >> 3;
  uint8_t y = (line + mmu->scrollY) & 0x7;
  uint8_t x = mmu->scrollX & 0x7;

  uint8_t tile = mmu->vramread(lineStartOffset + lineTile);
  if (tileset == 0) {
    if (tile < 128) {
      tile += 128;
    } else {
      // TODO: this may need to invert the tile ordering
    }
  }
  uint16_t tileData = interleave(
    mmu->vramread(tilesetOffset + (tile * 16) + (y * 2)),
    mmu->vramread(tilesetOffset + (tile * 16) + (y * 2) + 1)
  );

  uint32_t line_off = line * LINE_WIDTH;
  for (int i = 0; i < LINE_WIDTH; i++) {
    uint8_t tileColor = (tileData >> (x * 2)) & 0x3;
    framebuffer[line_off + i] = palette.entries[tileColor];
    x++;
    if (x >= 8) {
      x = 0;
      lineTile = (lineTile + 1);
      tile = mmu->vramread(lineStartOffset + lineTile);
      if (tileset == 0) {
        if (tile < 128) {
          tile += 128;
        } else {
          // TODO: this may need to invert the tile ordering
        }
      }
      tileData = interleave(
        mmu->vramread(tilesetOffset + (tile * 16) + (y * 2)),
        mmu->vramread(tilesetOffset + (tile * 16) + (y * 2) + 1)
      );
    }
  }

}
void GPU::writeToVsyncBuffer() {
  memcpy(vsyncBuffer.data(), framebuffer.data(), vsyncBuffer.size());
}
