#ifndef PGB_GB_MODE_HPP
#define PGB_GB_MODE_HPP

enum class GBMode {
  /** Original game boy (aka dmg/dot matrix game) */
  GB = 0,
  /** Game boy pocket/game boy light, also MGB */
  GBP,
  /** Game boy color */
  GBC,
  /** Super game boy/super game boy 2 */
  SGB,
  /** Game boy advance */
  GBA,
  /** Game boy advance SP */
  GBASP
};

#endif //PGB_GB_MODE_HPP
