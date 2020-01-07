#include "interpreter.hpp"

namespace interpreter {

// Template functions won't inline properly sooo macros (much faster)

#define jump(addr) do{\
  cpu->pc(addr);\
  cpu->clock(4);\
} while(0)

#define inc_rr(reg) do {\
  uint16_t r = cpu->reg();\
  cpu->reg(r + 1);\
  cpu->clock(4);\
} while (0)

#define dec_rr(reg) do {\
  uint16_t r = cpu->reg();\
  cpu->reg(r - 1);\
  cpu->clock(4);\
} while (0)

#define inc_r(reg) do {\
  uint8_t r = cpu->reg();\
  uint8_t res = r + 1;\
  uint8_t halfRes = (r & 0x0Fu) + 1;\
  cpu->b(res);\
  \
  cpu->zero(res == 0);\
  cpu->sub(false);\
  cpu->halfCarry(halfRes > 0xF0u);\
} while (0)

#define dec_r(reg) do {\
  uint8_t r = cpu->reg();\
  uint8_t res = r - 1;\
  uint8_t halfRes = (r & 0x0Fu) - 1;\
  cpu->b(res);\
  \
  cpu->zero(res == 0);\
  cpu->sub(false);\
  cpu->halfCarry(halfRes > 0xF0u);\
} while (0)

#define ld_rr_nn(reg) do {\
  uint16_t imm = cpu->pcRead16();\
  cpu->bc(imm);\
} while (0)

#define add_hl_rr(reg) do{\
  uint16_t hl = cpu->hl();\
  uint16_t r = cpu->reg();\
  uint32_t add = hl + r;\
  uint16_t halfAdd = (hl & 0xFFu) + (r & 0xFFu);\
  \
  cpu->sub(false);\
  cpu->halfCarry(halfAdd > 0xFFu);\
  cpu->carry(add > 0xFFFFu);\
  \
  cpu->hl(add);\
  cpu->clock(4);\
} while (0)

#define jr_n() do{\
  int8_t offset = static_cast<int8_t>(cpu->pcRead8());\
  jump(cpu->pc() + offset);\
} while (0)

#define add_a_value(value) do {\
  uint8_t a = cpu->a();\
  uint8_t v = value;\
  uint16_t sum = a + v;\
  uint8_t halfSum = (a & 0xFu) + (v & 0xFu);\
  cpu->zero(sum == 0u);\
  cpu->sub(false);\
  cpu->halfCarry(halfSum > 0x0Fu);\
  cpu->carry(sum > 0xFFu);\
  cpu->a(sum);\
} while(0)

#define adc_a_value(value) do {\
  uint8_t a = cpu->a();\
  uint8_t v = value;\
  uint8_t carry = cpu->carry() ? 1 : 0;\
  uint16_t sum = a + v + carry;\
  uint8_t halfSum = (a & 0xFu) + (v & 0xFu) + carry;\
  cpu->zero(sum == 0u);\
  cpu->sub(false);\
  cpu->halfCarry(halfSum > 0x0Fu);\
  cpu->carry(sum > 0xFFu);\
  cpu->a(sum);\
} while(0)

#define sub_a_value(value) do {\
  uint8_t a = cpu->a();\
  uint8_t v = value;\
  uint16_t diff = a - v;\
  uint8_t halfa = (a & 0xFu);\
  uint8_t halfv = (v * 0xFu);\
  cpu->zero(diff == 0u);\
  cpu->sub(true);\
  cpu->halfCarry(halfv > halfa);\
  cpu->carry(v > a);\
  cpu->a(diff);\
} while(0)

#define sbc_a_value(value) do {\
  uint8_t a = cpu->a();\
  uint8_t v = value;\
  uint8_t carry = cpu->carry() ? 1 : 0;\
  uint16_t diff = a - v - carry;\
  uint8_t halfa = (a & 0xFu);\
  uint8_t halfv = (v * 0xFu);\
  cpu->zero(diff == 0u);\
  cpu->sub(true);\
  cpu->halfCarry(halfv > halfa + carry);\
  cpu->carry(v > a + carry);\
  cpu->a(diff);\
} while(0)

#define and_a_value(value) do {\
  uint8_t a = cpu->a();\
  uint8_t v = value;\
  uint16_t aand = a & v;\
  cpu->zero(aand == 0u);\
  cpu->sub(false);\
  cpu->halfCarry(true);\
  cpu->carry(false);\
  cpu->a(aand);\
} while(0)

#define xor_a_value(value) do {\
  uint8_t a = cpu->a();\
  uint8_t v = value;\
  uint16_t xxor = a ^ v;\
  cpu->zero(xxor == 0u);\
  cpu->sub(false);\
  cpu->halfCarry(false);\
  cpu->carry(false);\
  cpu->a(xxor);\
} while(0)

#define or_a_value(value) do {\
  uint8_t a = cpu->a();\
  uint8_t v = value;\
  uint16_t oor = a | v;\
  cpu->zero(oor == 0u);\
  cpu->sub(false);\
  cpu->halfCarry(false);\
  cpu->carry(false);\
  cpu->a(oor);\
} while(0)

#define cp_a_value(value) do {\
  uint8_t a = cpu->a();\
  uint8_t v = value;\
  uint8_t halfa = (a & 0xFu);\
  uint8_t halfv = (v * 0xFu);\
  cpu->zero(a == v);\
  cpu->sub(true);\
  cpu->halfCarry(halfv > halfa);\
  cpu->carry(v > a);\
} while(0)

void op_00(CPU __unused *cpu) {
  // nop
}

void op_01(CPU *cpu) {
  // ld bc, nn
  ld_rr_nn(bc);
}

void op_02(CPU *cpu) {
  // ld bc, a
  cpu->write8(cpu->bc(), cpu->a());
}

void op_03(CPU *cpu) {
  // inc bc
  inc_rr(bc);
}

void op_04(CPU *cpu) {
  // inc b
  inc_r(b);
}

void op_05(CPU *cpu) {
  // dec b
  dec_r(b);
}

void op_06(CPU *cpu) {
  // ld b, n
  cpu->b(cpu->pcRead8());
}

void op_07(CPU *cpu) {
  // rlca
  uint8_t a = cpu->a();

  cpu->zero(false);
  cpu->sub(false);
  cpu->halfCarry(false);
  cpu->carry((a & 0x80u) != 0);

  cpu->a((a << 1u) | (a >> 7u));
}

void op_08(CPU *cpu) {
  // ld (nn), sp
  uint16_t addr = cpu->pcRead16();
  cpu->write16(addr, cpu->sp());
}

void op_09(CPU *cpu) {
  // add hl, bc
  add_hl_rr(bc);
}

void op_0a(CPU *cpu) {
  // ld a, (bc)
  cpu->a(cpu->read16(cpu->bc()));
}

void op_0b(CPU *cpu) {
  // dec bc
  dec_rr(bc);
}

void op_0c(CPU *cpu) {
  // inc c
  inc_r(c);
}

void op_0d(CPU *cpu) {
  // dec c
  dec_r(c);
}

void op_0e(CPU *cpu) {
  // ld c, n
  cpu->c(cpu->pcRead8());
}

void op_0f(CPU *cpu) {
  // rrca
  uint8_t a = cpu->a();

  cpu->zero(false);
  cpu->sub(false);
  cpu->halfCarry(false);
  cpu->carry((a & 1u) != 0);

  cpu->a(a >> 1u);
}

void op_10(CPU *cpu) {
  op_00(cpu); // TODO: Stop
}

void op_11(CPU *cpu) {
  // ld de, nn
  ld_rr_nn(de);
}

void op_12(CPU *cpu) {
  // ld de, a
  cpu->write8(cpu->de(), cpu->a());
}

void op_13(CPU *cpu) {
  // inc de
  inc_rr(de);
}

void op_14(CPU *cpu) {
  // inc d
  inc_r(d);
}

void op_15(CPU *cpu) {
  // dec d
  dec_r(d);
}

void op_16(CPU *cpu) {
  // ld d, n
  cpu->d(cpu->pcRead8());
}

void op_17(CPU *cpu) {
  // rla
  uint8_t a = cpu->a();
  uint8_t carry = cpu->carry() ? 1 : 0;

  cpu->zero(false);
  cpu->sub(false);
  cpu->halfCarry(false);
  cpu->carry((a & 0x80u) != 0);

  cpu->a((a << 1u) | carry);
}

void op_18(CPU *cpu) {
  // jr disp
  // jr n
  jr_n();
}

void op_19(CPU *cpu) {
  // add hl, de
  add_hl_rr(de);
}

void op_1a(CPU *cpu) {
  // ld a, (de)
  cpu->a(cpu->read8(cpu->de()));
}

void op_1b(CPU *cpu) {
  // dec de
  dec_rr(de);
}

void op_1c(CPU *cpu) {
  // inc e
  inc_r(c);
}

void op_1d(CPU *cpu) {
  // dec e
  dec_r(c);
}

void op_1e(CPU *cpu) {
  // ld e, n
  cpu->e(cpu->pcRead8());
}

void op_1f(CPU *cpu) {
  // rra
  uint8_t a = cpu->a();
  uint8_t carry = cpu->carry() ? 0x80 : 0;

  cpu->zero(false);
  cpu->sub(false);
  cpu->halfCarry(false);
  cpu->carry((a & 1u) != 0);

  cpu->a((carry >> 1u) | carry);
}

void op_20(CPU *cpu) {
  // jr nz, n
  // jr nz, disp
  if (!cpu->zero()) {
    jr_n();
  } else {
    jump(cpu->pc() + 1);
  }
}

void op_21(CPU *cpu) {
  // ld hl, nn
  ld_rr_nn(hl);
}

void op_22(CPU *cpu) {
  // ld (hl+), a
  // ld (hli), a
  cpu->write8(cpu->hl(), cpu->a());
  cpu->hl(cpu->hl() + 1);
}

void op_23(CPU *cpu) {
  // inc hl
  inc_rr(hl);
}

void op_24(CPU *cpu) {
  // inc h
  inc_r(h);
}

void op_25(CPU *cpu) {
  // dec h
  dec_r(h);
}

void op_26(CPU *cpu) {
  // ld h, n
  cpu->h(cpu->pcRead8());
}

void op_27(CPU *cpu) {
  // daa
  op_00(cpu); // TODO daa
}

void op_28(CPU *cpu) {
  // jr z, n
  // jr z, disp
  if (cpu->carry()) {
    jr_n();
  } else {
    jump(cpu->pc() + 1);
  }
}

void op_29(CPU *cpu) {
  // add hl, hl
  add_hl_rr(hl);
}

void op_2a(CPU *cpu) {
  // ld a, (hl+)
  // ld a, (hli)
  cpu->a(cpu->read8(cpu->hl()));
  cpu->hl(cpu->hl() + 1);
}

void op_2b(CPU *cpu) {
  // dec hl
  dec_rr(hl);
}

void op_2c(CPU *cpu) {
  // inc l
  inc_r(l);
}

void op_2d(CPU *cpu) {
  // dec l
  dec_r(l);
}

void op_2e(CPU *cpu) {
  // ld l, n
  cpu->l(cpu->pcRead8());
}

void op_2f(CPU *cpu) {
  // cpl
  cpu->a(~cpu->a());
  cpu->sub(true);
  cpu->halfCarry(true);
}

void op_30(CPU *cpu) {
  // jr nc, n
  // jr nc, disp
  if (!cpu->carry()) {
    jr_n();
  } else {
    jump(cpu->pc() + 1);
  }
}

void op_31(CPU *cpu) {
  // ld sp, nn
  ld_rr_nn(sp);
}

void op_32(CPU *cpu) {
  // ld (hli), a
  // ld (hld), a
  cpu->write8(cpu->hl(), cpu->a());
  cpu->hl(cpu->hl() - 1);
}

void op_33(CPU *cpu) {
  // inc sp
  inc_rr(sp);
}

void op_34(CPU *cpu) {
  // inc (hl)
  uint8_t r = cpu->read8(cpu->hl());
  uint8_t res = r + 1;
  uint8_t halfRes = (r & 0x0Fu) + 1;
  cpu->write8(cpu->hl(), res);

  cpu->zero(res == 0);
  cpu->sub(false);
  cpu->halfCarry(halfRes > 0xF0u);
}

void op_35(CPU *cpu) {
  // dec (hl)
  uint8_t r = cpu->read8(cpu->hl());
  uint8_t res = r - 1;
  uint8_t halfRes = (r & 0x0Fu) - 1;
  cpu->write8(cpu->hl(), res);

  cpu->zero(res == 0);
  cpu->sub(false);
  cpu->halfCarry(halfRes > 0xF0u);
}

void op_36(CPU *cpu) {
  // ld (hl), n
  uint8_t imm = cpu->pcRead8();
  cpu->write8(cpu->hl(), imm);
}

void op_37(CPU *cpu) {
  // scf
  cpu->carry(true);
  cpu->halfCarry(false);
  cpu->sub(false);
}

void op_38(CPU *cpu) {
  // jr c, n
  // jr c, disp
  if (cpu->carry()) {
    jr_n();
  } else {
    jump(cpu->pc() + 1);
  }
}

void op_39(CPU *cpu) {
  // add hl, sp
  add_hl_rr(sp);
}

void op_3a(CPU *cpu) {
  // ld a, (hl-)
  // ld a, (hld)
  cpu->a(cpu->read8(cpu->hl()));
  cpu->hl(cpu->hl() - 1);
}

void op_3b(CPU *cpu) {
  // dec sp
  dec_rr(sp);
}

void op_3c(CPU *cpu) {
  // inc a
  inc_r(a);
}

void op_3d(CPU *cpu) {
  // dec a
  dec_r(a);
}

void op_3e(CPU *cpu) {
  // ld a, n
  cpu->a(cpu->pcRead8());
}

void op_3f(CPU *cpu) {
  //ccf
  cpu->carry(!cpu->carry());
  cpu->sub(false);
  cpu->halfCarry(false);
}

void op_40(CPU __unused *cpu) {
  // ld b, b
  // noop
}

void op_41(CPU *cpu) {
  // ld b, c
  cpu->b(cpu->c());
}

void op_42(CPU *cpu) {
  // ld b, d
  cpu->b(cpu->d());
}

void op_43(CPU *cpu) {
  // ld b, e
  cpu->b(cpu->e());
}

void op_44(CPU *cpu) {
  // ld b, h
  cpu->b(cpu->h());
}

void op_45(CPU *cpu) {
  // ld b, l
  cpu->b(cpu->l());
}

void op_46(CPU *cpu) {
  // ld b, (hl)
  cpu->b(cpu->read8(cpu->hl()));
}

void op_47(CPU *cpu) {
  // ld b, a
  cpu->b(cpu->a());
}

void op_48(CPU *cpu) {
  // ld c, b
  cpu->c(cpu->b());
}

void op_49(CPU __unused *cpu) {
  // ld c, c
  // noop
}

void op_4a(CPU *cpu) {
  // ld c, d
  cpu->c(cpu->d());
}

void op_4b(CPU *cpu) {
  // ld c, e
  cpu->c(cpu->e());
}

void op_4c(CPU *cpu) {
  // ld c, h
  cpu->c(cpu->h());
}

void op_4d(CPU *cpu) {
  // ld c, l
  cpu->c(cpu->l());
}

void op_4e(CPU *cpu) {
  // ld c, (hl)
  cpu->c(cpu->read8(cpu->hl()));
}

void op_4f(CPU *cpu) {
  // ld c, a
  cpu->c(cpu->a());
}

void op_50(CPU *cpu) {
  // ld d, b
  cpu->d(cpu->b());
}

void op_51(CPU *cpu) {
  // ld d, c
  cpu->d(cpu->c());
}

void op_52(CPU __unused *cpu) {
  // ld d, b
  // noop
}

void op_53(CPU *cpu) {
  // ld d, e
  cpu->d(cpu->e());
}

void op_54(CPU *cpu) {
  // ld d, h
  cpu->d(cpu->h());
}

void op_55(CPU *cpu) {
  // ld d, l
  cpu->d(cpu->l());
}

void op_56(CPU *cpu) {
  // ld d, (hl)
  cpu->d(cpu->read8(cpu->hl()));
}

void op_57(CPU *cpu) {
  // ld d, a
  cpu->d(cpu->a());
}

void op_58(CPU *cpu) {
  // ld e, b
  cpu->e(cpu->b());
}

void op_59(CPU *cpu) {
  // ld e, c
  cpu->e(cpu->c());
}

void op_5a(CPU *cpu) {
  // ld e, d
  cpu->e(cpu->d());
}

void op_5b(CPU __unused *cpu) {
  // ld e, e
  // noop
}

void op_5c(CPU *cpu) {
  // ld e, h
  cpu->e(cpu->h());
}

void op_5d(CPU *cpu) {
  // ld e, l
  cpu->e(cpu->l());
}

void op_5e(CPU *cpu) {
  // ld e, (hl)
  cpu->e(cpu->read8(cpu->hl()));
}

void op_5f(CPU *cpu) {
  // ld e, a
  cpu->e(cpu->a());
}

void op_60(CPU *cpu) {
  // ld h, b
  cpu->h(cpu->b());
}

void op_61(CPU *cpu) {
  // ld h, c
  cpu->h(cpu->c());
}

void op_62(CPU *cpu) {
  // ld h, d
  cpu->d(cpu->d());
}

void op_63(CPU *cpu) {
  // ld h, e
  cpu->h(cpu->e());
}

void op_64(CPU __unused *cpu) {
  // ld h, h
  // noop
}

void op_65(CPU *cpu) {
  // ld h, l
  cpu->h(cpu->l());
}

void op_66(CPU *cpu) {
  // ld h, (hl)
  cpu->h(cpu->read8(cpu->hl()));
}

void op_67(CPU *cpu) {
  // ld h, b
  cpu->h(cpu->a());
}

void op_68(CPU *cpu) {
  // ld l, b
  cpu->l(cpu->b());
}

void op_69(CPU *cpu) {
  // ld l, c
  cpu->l(cpu->c());
}

void op_6a(CPU *cpu) {
  // ld l, d
  cpu->l(cpu->d());
}

void op_6b(CPU *cpu) {
  // ld l, e
  cpu->l(cpu->e());
}

void op_6c(CPU *cpu) {
  // ld l, h
  cpu->l(cpu->h());
}

void op_6d(CPU __unused *cpu) {
  // ld l, l
  // noop
}

void op_6e(CPU *cpu) {
  // ld l, (hl)
  cpu->l(cpu->read8(cpu->hl()));
}

void op_6f(CPU *cpu) {
  // ld l, a
  cpu->l(cpu->a());
}

void op_70(CPU *cpu) {
  // ld (hl), b
  cpu->write8(cpu->h(), cpu->b());
}

void op_71(CPU *cpu) {
  // ld (hl), c
  cpu->write8(cpu->h(), cpu->c());
}

void op_72(CPU *cpu) {
  // ld (hl), d
  cpu->write8(cpu->h(), cpu->d());
}

void op_73(CPU *cpu) {
  // ld (hl), e
  cpu->write8(cpu->h(), cpu->e());
}

void op_74(CPU *cpu) {
  // ld (hl), b
  cpu->write8(cpu->h(), cpu->h());
}

void op_75(CPU *cpu) {
  // ld (hl), l
  cpu->write8(cpu->h(), cpu->l());
}

void op_76(CPU *cpu) {
  // halt
  op_00(cpu); // TODO halt
}

void op_77(CPU *cpu) {
  // ld (hl), a
  cpu->write8(cpu->h(), cpu->a());
}

void op_78(CPU *cpu) {
  // ld a, b
  cpu->a(cpu->b());
}

void op_79(CPU *cpu) {
  // ld a, c
  cpu->a(cpu->c());
}

void op_7a(CPU *cpu) {
  // ld a, d
  cpu->a(cpu->d());
}

void op_7b(CPU *cpu) {
  // ld a, e
  cpu->a(cpu->e());
}

void op_7c(CPU *cpu) {
  // ld a, h
  cpu->a(cpu->h());
}

void op_7d(CPU *cpu) {
  // ld a, l
  cpu->a(cpu->l());
}

void op_7e(CPU *cpu) {
  // ld a, (hl)
  cpu->a(cpu->read8(cpu->hl()));
}

void op_7f(CPU __unused *cpu) {
  // ld a, a
  // noop
}

void op_80(CPU *cpu) {
  // add a, b
  add_a_value(cpu->b());
}

void op_81(CPU *cpu) {
  // add a, c
  add_a_value(cpu->c());
}

void op_82(CPU *cpu) {
  // add a, d
  add_a_value(cpu->d());
}

void op_83(CPU *cpu) {
  // add a, e
  add_a_value(cpu->e());
}

void op_84(CPU *cpu) {
  // add a, h
  add_a_value(cpu->h());
}

void op_85(CPU *cpu) {
  // add a, l
  add_a_value(cpu->l());
}

void op_86(CPU *cpu) {
  // add a, (hl)
  add_a_value(cpu->read8(cpu->hl()));
}

void op_87(CPU *cpu) {
  // add a, a
  add_a_value(cpu->a());
}

void op_88(CPU *cpu) {
  // adc a, b
  adc_a_value(cpu->b());
}

void op_89(CPU *cpu) {
  // adc a, c
  adc_a_value(cpu->c());
}

void op_8a(CPU *cpu) {
  // adc a, d
  adc_a_value(cpu->d());
}

void op_8b(CPU *cpu) {
  // adc a, e
  adc_a_value(cpu->e());
}

void op_8c(CPU *cpu) {
  // adc a, h
  adc_a_value(cpu->h());
}

void op_8d(CPU *cpu) {
  // adc a, l
  adc_a_value(cpu->l());
}

void op_8e(CPU *cpu) {
  // adc a, (hl)
  adc_a_value(cpu->read8(cpu->hl()));
}

void op_8f(CPU *cpu) {
  // adc a, a
  adc_a_value(cpu->a());
}

void op_90(CPU *cpu) {
  // sub a, b
  // sub b
  sub_a_value(cpu->b());
}

void op_91(CPU *cpu) {
  // sub a, c
  // sub c
  sub_a_value(cpu->c());
}

void op_92(CPU *cpu) {
  // sub a, d
  // sub d
  sub_a_value(cpu->d());
}

void op_93(CPU *cpu) {
  // sub a, e
  // sub e
  sub_a_value(cpu->e());
}

void op_94(CPU *cpu) {
  // sub a, h
  // sub h
  sub_a_value(cpu->h());
}

void op_95(CPU *cpu) {
  // sub a, l
  // sub l
  sub_a_value(cpu->l());
}

void op_96(CPU *cpu) {
  // sub a, b
  // sub b
  sub_a_value(cpu->read8(cpu->hl()));
}

void op_97(CPU *cpu) {
  // sub a, a
  // sub a
  sub_a_value(cpu->a());
}

void op_98(CPU *cpu) {
  // sbc a, b
  sbc_a_value(cpu->b());
}

void op_99(CPU *cpu) {
  // sbc a, c
  sbc_a_value(cpu->c());
}

void op_9a(CPU *cpu) {
  // sbc a, d
  sbc_a_value(cpu->d());
}

void op_9b(CPU *cpu) {
  // sbc a, e
  sbc_a_value(cpu->e());
}

void op_9c(CPU *cpu) {
  // sbc a, h
  sbc_a_value(cpu->h());
}

void op_9d(CPU *cpu) {
  // sbc a, l
  sbc_a_value(cpu->l());
}

void op_9e(CPU *cpu) {
  // sbc a, (hl)
  sbc_a_value(cpu->read8(cpu->hl()));
}

void op_9f(CPU *cpu) {
  // sbc a, a
  sbc_a_value(cpu->a());
}

void op_a0(CPU *cpu) {
  // and a, b
  // and b
  and_a_value(cpu->b());
}

void op_a1(CPU *cpu) {
  // and a, c
  // and c
  and_a_value(cpu->c());
}

void op_a2(CPU *cpu) {
  // and a, d
  // and d
  and_a_value(cpu->d());
}

void op_a3(CPU *cpu) {
  // and a, e
  // and e
  and_a_value(cpu->e());
}

void op_a4(CPU *cpu) {
  // and a, h
  // and h
  and_a_value(cpu->h());
}

void op_a5(CPU *cpu) {
  // and a, l
  // and l
  and_a_value(cpu->l());
}

void op_a6(CPU *cpu) {
  // and a, (hl)
  // and (hl)
  and_a_value(cpu->read8(cpu->hl()));
}

void op_a7(CPU *cpu) {
  // and a, a
  // and a
  and_a_value(cpu->a());
}

void op_a8(CPU *cpu) {
  // xor a, b
  // xor b
  xor_a_value(cpu->b());
}

void op_a9(CPU *cpu) {
  // xor a, c
  // xor c
  xor_a_value(cpu->c());
}

void op_aa(CPU *cpu) {
  // xor a, d
  // xor d
  xor_a_value(cpu->d());
}

void op_ab(CPU *cpu) {
  // xor a, e
  // xor e
  xor_a_value(cpu->e());
}

void op_ac(CPU *cpu) {
  // xor a, h
  // xor h
  xor_a_value(cpu->h());
}

void op_ad(CPU *cpu) {
  // xor a, l
  // xor l
  xor_a_value(cpu->l());
}

void op_ae(CPU *cpu) {
  // xor a, (hl)
  // xor (hl)
  xor_a_value(cpu->read8(cpu->hl()));
}

void op_af(CPU *cpu) {
  // xor a, a
  // xor a
  xor_a_value(cpu->a());
}

void op_b0(CPU *cpu) {
  // or a, b
  // or b
  or_a_value(cpu->b());
}

void op_b1(CPU *cpu) {
  // or a, c
  // or c
  or_a_value(cpu->c());
}

void op_b2(CPU *cpu) {
  // or a, d
  // or d
  or_a_value(cpu->d());
}

void op_b3(CPU *cpu) {
  // or a, e
  // or e
  or_a_value(cpu->e());
}

void op_b4(CPU *cpu) {
  // or a, h
  // or h
  or_a_value(cpu->h());
}

void op_b5(CPU *cpu) {
  // or a, l
  // or l
  or_a_value(cpu->l());
}

void op_b6(CPU *cpu) {
  // or a, (hl)
  // or (hl)
  or_a_value(cpu->read8(cpu->hl()));
}

void op_b7(CPU *cpu) {
  // or a, a
  // or a
  or_a_value(cpu->a());
}

void op_b8(CPU *cpu) {
  // cp a, b
  // cp b
  cp_a_value(cpu->b());
}

void op_b9(CPU *cpu) {
  // cp a, c
  // cp c
  cp_a_value(cpu->c());
}

void op_ba(CPU *cpu) {
  // cp a, d
  // cp d
  cp_a_value(cpu->d());
}

void op_bb(CPU *cpu) {
  // cp a, e
  // cp e
  cp_a_value(cpu->e());
}

void op_bc(CPU *cpu) {
  // cp a, h
  // cp h
  cp_a_value(cpu->h());
}

void op_bd(CPU *cpu) {
  // cp a, l
  // cp l
  cp_a_value(cpu->l());
}

void op_be(CPU *cpu) {
  // cp a, (hl)
  // cp (hl)
  cp_a_value(cpu->read8(cpu->hl()));
}

void op_bf(CPU *cpu) {
  // cp a, a
  // cp a
  cp_a_value(cpu->a());
}

void op_c0(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_c1(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_c2(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_c3(CPU *cpu) {
  // jp a16
  uint16_t newPC = cpu->pcRead16();
  jump(newPC);
}

void op_c4(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_c5(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_c6(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_c7(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_c8(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_c9(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_ca(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_cb(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_cc(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_cd(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_ce(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_cf(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_d0(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_d1(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_d2(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_d3(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_d4(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_d5(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_d6(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_d7(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_d8(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_d9(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_da(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_db(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_dc(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_dd(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_de(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_df(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_e0(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_e1(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_e2(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_e3(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_e4(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_e5(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_e6(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_e7(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_e8(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_e9(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_ea(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_eb(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_ec(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_ed(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_ee(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_ef(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_f0(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_f1(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_f2(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_f3(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_f4(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_f5(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_f6(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_f7(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_f8(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_f9(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_fa(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_fb(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_fc(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_fd(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_fe(CPU *cpu) {
  op_00(cpu); // TODO
}

void op_ff(CPU *cpu) {
  op_00(cpu); // TODO
}

void (*ops[])(CPU *) = {
  op_00, op_01, op_02, op_03, op_04, op_05, op_06, op_07, op_08, op_09, op_0a, op_0b, op_0c, op_0d, op_0e, op_0f,
  op_10, op_11, op_12, op_13, op_14, op_15, op_16, op_17, op_18, op_19, op_1a, op_1b, op_1c, op_1d, op_1e, op_1f,
  op_20, op_21, op_22, op_23, op_24, op_25, op_26, op_27, op_28, op_29, op_2a, op_2b, op_2c, op_2d, op_2e, op_2f,
  op_30, op_31, op_32, op_33, op_34, op_35, op_36, op_37, op_38, op_39, op_3a, op_3b, op_3c, op_3d, op_3e, op_3f,
  op_40, op_41, op_42, op_43, op_44, op_45, op_46, op_47, op_48, op_49, op_4a, op_4b, op_4c, op_4d, op_4e, op_4f,
  op_50, op_51, op_52, op_53, op_54, op_55, op_56, op_57, op_58, op_59, op_5a, op_5b, op_5c, op_5d, op_5e, op_5f,
  op_60, op_61, op_62, op_63, op_64, op_65, op_66, op_67, op_68, op_69, op_6a, op_6b, op_6c, op_6d, op_6e, op_6f,
  op_70, op_71, op_72, op_73, op_74, op_75, op_76, op_77, op_78, op_79, op_7a, op_7b, op_7c, op_7d, op_7e, op_7f,
  op_80, op_81, op_82, op_83, op_84, op_85, op_86, op_87, op_88, op_89, op_8a, op_8b, op_8c, op_8d, op_8e, op_8f,
  op_90, op_91, op_92, op_93, op_94, op_95, op_96, op_97, op_98, op_99, op_9a, op_9b, op_9c, op_9d, op_9e, op_9f,
  op_a0, op_a1, op_a2, op_a3, op_a4, op_a5, op_a6, op_a7, op_a8, op_a9, op_aa, op_ab, op_ac, op_ad, op_ae, op_af,
  op_b0, op_b1, op_b2, op_b3, op_b4, op_b5, op_b6, op_b7, op_b8, op_b9, op_ba, op_bb, op_bc, op_bd, op_be, op_bf,
  op_c0, op_c1, op_c2, op_c3, op_c4, op_c5, op_c6, op_c7, op_c8, op_c9, op_ca, op_cb, op_cc, op_cd, op_ce, op_cf,
  op_d0, op_d1, op_d2, op_d3, op_d4, op_d5, op_d6, op_d7, op_d8, op_d9, op_da, op_db, op_dc, op_dd, op_de, op_df,
  op_e0, op_e1, op_e2, op_e3, op_e4, op_e5, op_e6, op_e7, op_e8, op_e9, op_ea, op_eb, op_ec, op_ed, op_ee, op_ef,
  op_f0, op_f1, op_f2, op_f3, op_f4, op_f5, op_f6, op_f7, op_f8, op_f9, op_fa, op_fb, op_fc, op_fd, op_fe, op_ff,
};

}
