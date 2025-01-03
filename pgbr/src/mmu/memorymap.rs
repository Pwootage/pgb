use crate::const_assert;

pub struct MemoryMap<const startAddr: u16, const endAddr: u16>();

impl<const startAddr: u16, const endAddr: u16> MemoryMap<startAddr, endAddr> {
  const_assert!(startAddr < endAddr);

  pub const fn start() -> u16 {
    startAddr
  }

  pub const fn end() -> u16 {
    endAddr
  }

  pub const fn size() -> usize {
    (endAddr - startAddr) as usize
  }

  pub const fn contains(addr: u16) -> bool {
    addr >= startAddr && addr < endAddr
  }

  pub const fn addr_is_below(addr: u16) -> bool {
    addr < startAddr
  }
}