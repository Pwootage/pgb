pub struct MemoryMap<const START_ADDR: u16, const END_ADDR: u16>();

impl<const START_ADDR: u16, const END_ADDR: u16> MemoryMap<START_ADDR, END_ADDR> {
  // TODO: this assert
  // const_assert!(START_ADDR < END_ADDR);

  pub const fn start() -> u16 {
    START_ADDR
  }

  pub const fn end() -> u16 {
    END_ADDR
  }

  pub const fn size() -> usize {
    (Self::end() - Self::start()) as usize
  }

  pub const fn contains(addr: u16) -> bool {
    addr >= START_ADDR && addr <= END_ADDR
  }

  // pub const fn addr_is_below(addr: u16) -> bool {
  //   addr < START_ADDR
  // }
}