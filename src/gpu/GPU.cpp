#include "GPU.hpp"

#include <utility>

GPU::GPU(std::shared_ptr<MMU> mmu) : mmu(std::move(mmu)) {}

void GPU::update(uint64_t clockDelta) {
  if (!mmu->lcdPower()) {
    gpuClock = 0;
    return;
  }

  // TODO: there might be a cleaner solution
  gpuClock += clockDelta;
  switch (mmu->gpu_mode) {
    case GPU_MODE::SCAN_OAM:
      if (gpuClock > CLOCK_SCANLINE_OAM) {
        gpuClock -= CLOCK_SCANLINE_OAM;
        mmu->gpu_mode = GPU_MODE::SCAN_VRAM;
      }
      break;
    case GPU_MODE::SCAN_VRAM:
      if (gpuClock > CLOCK_SCANLINE_VRAM) {
        gpuClock -= CLOCK_SCANLINE_VRAM;
        mmu->gpu_mode = GPU_MODE::HBLANK;
        renderLine();
      }
      break;
    case GPU_MODE::HBLANK:
      if (gpuClock > CLOCK_SCANLINE_HBLANK) {
        gpuClock -= CLOCK_SCANLINE_HBLANK;
        mmu->gpu_line++;
        if (mmu->gpu_line == LINES) {
          writeToVsyncBuffer();
          mmu->gpu_mode = GPU_MODE::VBLANK;
        } else {
          mmu->gpu_mode = GPU_MODE::SCAN_OAM;
        }
      }
      break;
    case GPU_MODE::VBLANK:
      if (gpuClock > CLOCK_SCANLINE) {
        gpuClock -= CLOCK_SCANLINE;
        mmu->gpu_line++;
        if (mmu->gpu_line > LINES + 10) {
          mmu->gpu_mode = GPU_MODE::SCAN_OAM;
          mmu->gpu_line = 0;
        }
      }
      break;
  }
}

void GPU::renderLine() {
  uint8_t tileset = mmu->lcdBGWindowTileset();
  uint16_t tilesetOffset = tileset == 0 ? 0x800 : 0x0;

  // add the offset to the beginning of the line where we care

  uint16_t line = mmu->gpu_line + mmu->scrollY;
  uint16_t col = mmu->scrollX;

  uint8_t tile_y = line / 8;
  uint8_t tile_x = col / 8;
  uint8_t tile_relative_y = line % 8;
  uint8_t tile_relative_x = col % 8;

  uint16_t tile_y_offset = tile_y * (32);

  uint16_t lineStartOffset = mmu->lcdBGTileMap() == 0 ? 0x1800 : 0x1c00;
  lineStartOffset += tile_y_offset;

  uint8_t tile = mmu->vramread(lineStartOffset + tile_x);
  if (tile != 0) {
    printf("");
  }
  if (tileset == 0) {
    if (tile < 128) {
      tile += 128;
    } else {
      // TODO: this may need to invert the tile ordering
    }
  }
  uint8_t a = mmu->vramread(tilesetOffset + (tile * 16) + (tile_relative_y * 2));
  uint8_t b = mmu->vramread(tilesetOffset + (tile * 16) + (tile_relative_y * 2) + 1);
  uint16_t tileData = interleave(
    a,
    b
  );

  if (tile != 128) {
    printf("");
  }

  uint32_t line_off = mmu->gpu_line * LINE_WIDTH;
  for (int i = 0; i < LINE_WIDTH; i++) {
    uint8_t tileColor = (tileData >> (14 - tile_relative_x * 2)) & 0x3;
    framebuffer[line_off + i] = palette.entries[tileColor];
    tile_relative_x++;
    if (tile_relative_x >= 8) {
      tile_relative_x = 0;
      tile_x++;
      tile = mmu->vramread(lineStartOffset + tile_x);
      if (tile != 0) {
        printf("");
      }
      if (tileset == 0) {
        if (tile < 128) {
          tile += 128;
        } else {
          // TODO: this may need to invert the tile ordering
        }
      }
      tileData = interleave(
        mmu->vramread(tilesetOffset + (tile * 16) + (tile_relative_y * 2)),
        mmu->vramread(tilesetOffset + (tile * 16) + (tile_relative_y * 2) + 1)
      );
    }
  }

}
void GPU::writeToVsyncBuffer() {
  memcpy(vsyncBuffer.data(), framebuffer.data(), vsyncBuffer.size() * sizeof(GPU_PALETTE_ENTRY));
}
