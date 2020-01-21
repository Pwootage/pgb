#ifndef PGB_CPU_HPP
#define PGB_CPU_HPP

#include <cstdint>
#include <utility>
#include "gb_mode.hpp"
#include "MMU.hpp"

class CPU {
public:
  explicit CPU(std::shared_ptr<MMU> mmu);

  void printState();

  inline uint16_t af() { return _af; }
  inline void af(uint16_t value) { _af = value; }
  inline uint8_t a() { return (_af >> 8u) & 0xFFu; }
  inline void a(uint8_t value) { _af = (_af & 0xFFu) | (value << 8u); }
  inline uint8_t f() { return _af & 0xFFu; }
  inline void f(uint8_t value) { _af = (_af & 0xFF00u) | (value & 0xF0u); }

  inline uint16_t bc() { return _bc; }
  inline void bc(uint16_t value) { _bc = value; }
  inline uint8_t b() { return (_bc >> 8u) & 0xFFu; }
  inline void b(uint8_t value) { _bc = (_bc & 0xFFu) | (value << 8u); }
  inline uint8_t c() { return _bc & 0xFFu; }
  inline void c(uint8_t value) { _bc = (_bc & 0xFF00u) | value; }

  inline uint16_t de() { return _de; }
  inline void de(uint16_t value) { _de = value; }
  inline uint8_t d() { return (_de >> 8u) & 0xFFu; }
  inline void d(uint8_t value) { _de = (_de & 0xFFu) | (value << 8u); }
  inline uint8_t e() { return _de & 0xFFu; }
  inline void e(uint8_t value) { _de = (_de & 0xFF00u) | value; }

  inline uint16_t hl() { return _hl; }
  inline void hl(uint16_t value) { _hl = value; }
  inline uint8_t h() { return (_hl >> 8u) & 0xFFu; }
  inline void h(uint8_t value) { _hl = (_hl & 0xFFu) | (value << 8u); }
  inline uint8_t l() { return _hl & 0xFFu; }
  inline void l(uint8_t value) { _hl = (_hl & 0xFF00u) | value; }

  inline uint16_t sp() { return _sp; }
  inline void sp(uint16_t value) { _sp = value; }

  inline uint16_t pc() { return _pc; }
  inline void pc(uint16_t value) { _pc = value; }

  inline bool zero() { return (_af & 0x0080u) != 0; }
  inline void zero(bool zero) {
    if (zero) {
      _af |= 0x0080u;
    } else {
      _af &= 0xFF7Fu;
    }
  }

  inline bool sub() { return (_af & 0x0040u) != 0; }
  inline void sub(bool addSub) {
    if (addSub) {
      _af |= 0x0040u;
    } else {
      _af &= 0xFFBFu;
    }
  }

  inline bool halfCarry() { return (_af & 0x0020u) != 0; }
  inline void halfCarry(bool carry) {
    if (carry) {
      _af |= 0x0020u;
    } else {
      _af &= 0xFFDFu;
    }
  }


  inline bool carry() { return (_af & 0x0010u) != 0; }
  inline void carry(bool carry) {
    if (carry) {
      _af |= 0x0010u;
    } else {
      _af &= 0xFFEFu;
    }
  }

  inline void clock(uint8_t cycles) { _clock += cycles; }
  inline uint64_t clock() { return _clock; }

  inline uint8_t pcRead8() {
    uint8_t value = read8(pc());
    pc(pc() + 1);
    return value;
  }

  inline uint16_t pcRead16() {
    uint16_t value = read16(pc());
    pc(pc() + 2);
    return value;
  }

  inline uint8_t read8(uint16_t addr) {
    uint8_t value = mmu->read8(addr);
    clock(4);
    return value;
  }

  inline uint16_t read16(uint16_t addr) {
    uint16_t value = mmu->read16(addr);
    clock(8);
    return value;
  }

  inline void write8(uint16_t addr, uint8_t value) {
    mmu->write8(addr, value);
    clock(4);
  }

  inline void write16(uint16_t addr, uint16_t value) {
    mmu->write16(addr, value);
    clock(8);
  }

  void emulateInstruction();

  void freeze();

  void enableInterrupts();
  void disableInterrupts();

  std::array<uint64_t, 256> instrUsages{};
private:
  void initializeRegisters();

  std::shared_ptr<MMU> mmu;

  uint16_t _af = 0x01B0;
  uint16_t _bc = 0x0000;
  uint16_t _de = 0x00D8;
  uint16_t _hl = 0x014D;
  uint16_t _sp = 0xFFFE;
  uint16_t _pc = 0x0100;

  uint64_t _clock = 0;

  uint16_t last_clock_m = 0;
  uint16_t last_clock_t = 0;
};

#endif //PGB_CPU_HPP
