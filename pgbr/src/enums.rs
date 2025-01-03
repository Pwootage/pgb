#[derive(Debug, PartialEq, Copy, Clone)]
pub enum GpuMode {
  ScanOAM = 2,
  ScanVRAM = 3,
  HBlank = 0,
  VBlank = 1,
}

#[derive(Debug, PartialEq, Copy, Clone)]
pub enum BackgroundMode {
  BG,
  WINDOW,
}

#[derive(Debug, PartialEq, Copy, Clone)]
pub enum GBModel {
  /** Original game boy (aka dmg/dot matrix game) */
  GB = 0,
  /** Game boy pocket/game boy light, also MGB */
  GBP,
  /** Super game boy */
  SGB,
  /** Super game boy 2 */
  SGB2,
  /** Game boy color */
  GBC,
  /** Game boy advance */
  GBA,
  /** Game boy advance SP */
  GBASP
}

impl GBModel {
  pub fn has_banked_ram(&self) -> bool {
    match self {
      GBModel::GBC | GBModel::GBA | GBModel::GBASP => true,
      _ => false
    }
  }
}