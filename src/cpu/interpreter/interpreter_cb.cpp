#include "interpreter_cb.hpp"

namespace interpreter_cb {

#define bit_n(bit, value) do {\
  uint8_t v = (value);\
  uint8_t mask = 1 << bit;\
  cpu->zero((v & mask) != 0);\
  cpu->sub(false);\
  cpu->halfCarry(1);\
} while(0)

#define set_r(bit, reg) do {\
  uint8_t mask = 1u << bit;\
  cpu->reg(cpu->reg() | mask);\
} while(0)

#define set_hl(bit) do {\
  uint8_t mask = 1u << bit;\
  cpu->write8(cpu->hl(), cpu->read8(cpu->hl()) | mask);\
} while(0)

#define res_r(bit, reg) do {\
  uint8_t mask = 1u << bit;\
  mask = ~mask;\
  cpu->reg(cpu->reg() & mask);\
} while(0)

#define res_hl(bit) do {\
  uint8_t mask = 1u << bit;\
  mask = ~mask;\
  cpu->write8(cpu->hl(), cpu->read8(cpu->hl()) & mask);\
} while(0)

#define rlc() \
  uint8_t carry = (v & 0x80) >> 7u;\
  uint8_t res = (v << 1) | carry;\
  cpu->zero(res == 0);\
  cpu->sub(false);\
  cpu->halfCarry(false);\
  cpu->carry(carry != 0);

#define rl() \
  uint8_t oldCarry = cpu->carry() ? 1u : 0u;\
  bool carry = (v & 0x80) != 0;\
  uint8_t res = (v << 1) | oldCarry;\
  cpu->zero(res == 0);\
  cpu->sub(false);\
  cpu->halfCarry(false);\
  cpu->carry(carry);

#define rrc() \
  uint8_t carry = (v & 0x1) << 7u;\
  uint8_t res = (v >> 1) | carry;\
  cpu->zero(res == 0);\
  cpu->sub(false);\
  cpu->halfCarry(false);\
  cpu->carry(carry != 0);

#define rr() \
  uint8_t oldCarry = cpu->carry() ? 0x80u : 0u;\
  bool carry = (v & 0x1) != 0;\
  uint8_t res = (v >> 1) | oldCarry;\
  cpu->zero(res == 0);\
  cpu->sub(false);\
  cpu->halfCarry(false);\
  cpu->carry(carry);

#define sla() \
  uint8_t carry = (v & 0x80);\
  uint8_t res = (v << 1);\
  cpu->zero(res == 0);\
  cpu->sub(false);\
  cpu->halfCarry(false);\
  cpu->carry(carry != 0);

#define sra() \
  uint8_t carry = (v & 0x1);\
  uint8_t msb = (v & 0x80);\
  uint8_t res = (v >> 1u) | msb;\
  cpu->zero(res == 0);\
  cpu->sub(false);\
  cpu->halfCarry(false);\
  cpu->carry(carry != 0);

#define srl() \
  uint8_t carry = (v & 0x1);\
  uint8_t res = (v >> 1u);\
  cpu->zero(res == 0);\
  cpu->sub(false);\
  cpu->halfCarry(false);\
  cpu->carry(carry != 0);

#define swap() \
  uint8_t high = (v & 0xF0);\
  uint8_t low = (v & 0xF0);\
  uint8_t res = (high >> 4u) | (low << 4u);\
  cpu->zero(res == 0);\
  cpu->sub(false);\
  cpu->halfCarry(false);\
  cpu->carry(false);

void cb_00(CPU *cpu) {
  //rlc b
  uint8_t v = cpu->b();
  rlc()
  cpu->b(res);
}

void cb_01(CPU *cpu) {
  //rlc c
  uint8_t v = cpu->c();
  rlc()
  cpu->c(res);
}

void cb_02(CPU *cpu) {
  //rlc d
  uint8_t v = cpu->d();
  rlc()
  cpu->d(res);
}

void cb_03(CPU *cpu) {
  //rlc e
  uint8_t v = cpu->e();
  rlc()
  cpu->e(res);
}

void cb_04(CPU *cpu) {
  //rlc h
  uint8_t v = cpu->h();
  rlc()
  cpu->h(res);
}

void cb_05(CPU *cpu) {
  //rlc l
  uint8_t v = cpu->l();
  rlc()
  cpu->l(res);
}

void cb_06(CPU *cpu) {
  //rlc (hl)
  uint8_t v = cpu->read8(cpu->hl());
  rlc()
  cpu->write8(cpu->hl(), res);
}

void cb_07(CPU *cpu) {
  //rlc a
  uint8_t v = cpu->a();
  rlc()
  cpu->a(res);
}

void cb_08(CPU *cpu) {
  //rrc b
  uint8_t v = cpu->b();
  rrc()
  cpu->b(res);
}

void cb_09(CPU *cpu) {
  //rrc c
  uint8_t v = cpu->c();
  rrc()
  cpu->c(res);
}

void cb_0a(CPU *cpu) {
  //rrc d
  uint8_t v = cpu->d();
  rrc()
  cpu->d(res);
}

void cb_0b(CPU *cpu) {
  //rrc e
  uint8_t v = cpu->e();
  rrc()
  cpu->e(res);
}

void cb_0c(CPU *cpu) {
  //rrc h
  uint8_t v = cpu->h();
  rrc()
  cpu->h(res);
}

void cb_0d(CPU *cpu) {
  //rrc l
  uint8_t v = cpu->l();
  rrc()
  cpu->l(res);
}

void cb_0e(CPU *cpu) {
  //rrc (hl)
  uint8_t v = cpu->read8(cpu->hl());
  rrc()
  cpu->write8(cpu->hl(), res);
}

void cb_0f(CPU *cpu) {
  //rrc a
  uint8_t v = cpu->a();
  rrc()
  cpu->a(res);
}

void cb_10(CPU *cpu) {
  //rl b
  uint8_t v = cpu->b();
  rl()
  cpu->b(res);
}

void cb_11(CPU *cpu) {
  //rl c
  uint8_t v = cpu->c();
  rl()
  cpu->c(res);
}

void cb_12(CPU *cpu) {
  //rl d
  uint8_t v = cpu->d();
  rl()
  cpu->d(res);
}

void cb_13(CPU *cpu) {
  //rl e
  uint8_t v = cpu->e();
  rl()
  cpu->e(res);
}

void cb_14(CPU *cpu) {
  //rl h
  uint8_t v = cpu->h();
  rl()
  cpu->h(res);
}

void cb_15(CPU *cpu) {
  //rl l
  uint8_t v = cpu->l();
  rl()
  cpu->l(res);
}

void cb_16(CPU *cpu) {
  //rl (hl)
  uint8_t v = cpu->read8(cpu->hl());
  rl()
  cpu->write8(cpu->hl(), res);
}

void cb_17(CPU *cpu) {
  //rl a
  uint8_t v = cpu->a();
  rl()
  cpu->a(res);
}

void cb_18(CPU *cpu) {
  //rr b
  uint8_t v = cpu->b();
  rr()
  cpu->b(res);
}

void cb_19(CPU *cpu) {
  //rr c
  uint8_t v = cpu->c();
  rr()
  cpu->c(res);
}

void cb_1a(CPU *cpu) {
  //rr d
  uint8_t v = cpu->d();
  rr()
  cpu->d(res);
}

void cb_1b(CPU *cpu) {
  //rr e
  uint8_t v = cpu->e();
  rr()
  cpu->e(res);
}

void cb_1c(CPU *cpu) {
  //rr h
  uint8_t v = cpu->h();
  rr()
  cpu->h(res);
}

void cb_1d(CPU *cpu) {
  //rr l
  uint8_t v = cpu->l();
  rr()
  cpu->l(res);
}

void cb_1e(CPU *cpu) {
  //rr (hl)
  uint8_t v = cpu->read8(cpu->hl());
  rr()
  cpu->write8(cpu->hl(), res);
}

void cb_1f(CPU *cpu) {
  //rr a
  uint8_t v = cpu->a();
  rr()
  cpu->a(res);
}

void cb_20(CPU *cpu) {
  //sla b
  uint8_t v = cpu->b();
  sla()
  cpu->b(res);
}

void cb_21(CPU *cpu) {
  //sla c
  uint8_t v = cpu->c();
  sla()
  cpu->c(res);
}

void cb_22(CPU *cpu) {
  //sla d
  uint8_t v = cpu->d();
  sla()
  cpu->d(res);
}

void cb_23(CPU *cpu) {
  //sla e
  uint8_t v = cpu->e();
  sla()
  cpu->e(res);
}

void cb_24(CPU *cpu) {
  //sla h
  uint8_t v = cpu->h();
  sla()
  cpu->h(res);
}

void cb_25(CPU *cpu) {
  //sla l
  uint8_t v = cpu->l();
  sla()
  cpu->l(res);
}

void cb_26(CPU *cpu) {
  //sla (hl)
  uint8_t v = cpu->read8(cpu->hl());
  sla()
  cpu->write8(cpu->hl(), res);
}

void cb_27(CPU *cpu) {
  //sla a
  uint8_t v = cpu->a();
  sla()
  cpu->a(res);
}

void cb_28(CPU *cpu) {
  //sra b
  uint8_t v = cpu->b();
  sra()
  cpu->b(res);
}

void cb_29(CPU *cpu) {
  //sra c
  uint8_t v = cpu->c();
  sra()
  cpu->c(res);
}

void cb_2a(CPU *cpu) {
  //sra d
  uint8_t v = cpu->d();
  sra()
  cpu->d(res);
}

void cb_2b(CPU *cpu) {
  //sra e
  uint8_t v = cpu->e();
  sra()
  cpu->e(res);
}

void cb_2c(CPU *cpu) {
  //sra h
  uint8_t v = cpu->h();
  sra()
  cpu->h(res);
}

void cb_2d(CPU *cpu) {
  //sra l
  uint8_t v = cpu->l();
  sra()
  cpu->l(res);
}

void cb_2e(CPU *cpu) {
  //sra (hl)
  uint8_t v = cpu->read8(cpu->hl());
  sra()
  cpu->write8(cpu->hl(), res);
}

void cb_2f(CPU *cpu) {
  //sra a
  uint8_t v = cpu->a();
  sra()
  cpu->a(res);
}

void cb_30(CPU *cpu) {
  //swap b
  uint8_t v = cpu->b();
  swap()
  cpu->b(res);
}

void cb_31(CPU *cpu) {
  //swap c
  uint8_t v = cpu->c();
  swap()
  cpu->c(res);
}

void cb_32(CPU *cpu) {
  //swap d
  uint8_t v = cpu->d();
  swap()
  cpu->d(res);
}

void cb_33(CPU *cpu) {
  //swap e
  uint8_t v = cpu->e();
  swap()
  cpu->e(res);
}

void cb_34(CPU *cpu) {
  //swap h
  uint8_t v = cpu->h();
  swap()
  cpu->h(res);
}

void cb_35(CPU *cpu) {
  //swap l
  uint8_t v = cpu->l();
  swap()
  cpu->l(res);
}

void cb_36(CPU *cpu) {
  //swap (hl)
  uint8_t v = cpu->read8(cpu->hl());
  swap()
  cpu->write8(cpu->hl(), res);
}

void cb_37(CPU *cpu) {
  //swap a
  uint8_t v = cpu->a();
  swap()
  cpu->a(res);
}

void cb_38(CPU *cpu) {
  //srl b
  uint8_t v = cpu->b();
  srl()
  cpu->b(res);
}

void cb_39(CPU *cpu) {
  //srl c
  uint8_t v = cpu->c();
  srl()
  cpu->c(res);
}

void cb_3a(CPU *cpu) {
  //srl d
  uint8_t v = cpu->d();
  srl()
  cpu->d(res);
}

void cb_3b(CPU *cpu) {
  //srl e
  uint8_t v = cpu->e();
  srl()
  cpu->e(res);
}

void cb_3c(CPU *cpu) {
  //srl h
  uint8_t v = cpu->h();
  srl()
  cpu->h(res);
}

void cb_3d(CPU *cpu) {
  //srl l
  uint8_t v = cpu->l();
  srl()
  cpu->l(res);
}

void cb_3e(CPU *cpu) {
  //srl (hl)
  uint8_t v = cpu->read8(cpu->hl());
  srl()
  cpu->write8(cpu->hl(), res);
}

void cb_3f(CPU *cpu) {
  //srl a
  uint8_t v = cpu->a();
  srl()
  cpu->a(res);
}

void cb_40(CPU *cpu) {
  //bit 0, b
  bit_n(0, cpu->b());
}

void cb_41(CPU *cpu) {
  //bit 0, c
  bit_n(0, cpu->c());
}

void cb_42(CPU *cpu) {
  //bit 0, d
  bit_n(0, cpu->d());
}

void cb_43(CPU *cpu) {
  //bit 0, e
  bit_n(0, cpu->e());
}

void cb_44(CPU *cpu) {
  //bit 0, h
  bit_n(0, cpu->h());
}

void cb_45(CPU *cpu) {
  //bit 0, l
  bit_n(0, cpu->l());
}

void cb_46(CPU *cpu) {
  //bit 0, (hl)
  bit_n(0, cpu->read8(cpu->hl()));
}

void cb_47(CPU *cpu) {
  //bit 0, a
  bit_n(0, cpu->a());
}

void cb_48(CPU *cpu) {
  //bit 1, b
  bit_n(1, cpu->b());
}

void cb_49(CPU *cpu) {
  //bit 1, c
  bit_n(1, cpu->c());
}

void cb_4a(CPU *cpu) {
  //bit 1, d
  bit_n(1, cpu->d());
}

void cb_4b(CPU *cpu) {
  //bit 1, e
  bit_n(1, cpu->e());
}

void cb_4c(CPU *cpu) {
  //bit 1, h
  bit_n(1, cpu->h());
}

void cb_4d(CPU *cpu) {
  //bit 1, l
  bit_n(1, cpu->l());
}

void cb_4e(CPU *cpu) {
  //bit 1, (hl)
  bit_n(1, cpu->read8(cpu->hl()));
}

void cb_4f(CPU *cpu) {
  //bit 1, a
  bit_n(1, cpu->a());
}

void cb_50(CPU *cpu) {
  //bit 2, b
  bit_n(2, cpu->b());
}

void cb_51(CPU *cpu) {
  //bit 2, c
  bit_n(2, cpu->c());
}

void cb_52(CPU *cpu) {
  //bit 2, d
  bit_n(2, cpu->d());
}

void cb_53(CPU *cpu) {
  //bit 2, e
  bit_n(2, cpu->e());
}

void cb_54(CPU *cpu) {
  //bit 2, h
  bit_n(2, cpu->h());
}

void cb_55(CPU *cpu) {
  //bit 2, l
  bit_n(2, cpu->l());
}

void cb_56(CPU *cpu) {
  //bit 2, (hl)
  bit_n(2, cpu->read8(cpu->hl()));
}

void cb_57(CPU *cpu) {
  //bit 2, a
  bit_n(2, cpu->a());
}

void cb_58(CPU *cpu) {
  //bit 3, b
  bit_n(3, cpu->b());
}

void cb_59(CPU *cpu) {
  //bit 3, c
  bit_n(3, cpu->c());
}

void cb_5a(CPU *cpu) {
  //bit 3, d
  bit_n(3, cpu->d());
}

void cb_5b(CPU *cpu) {
  //bit 3, e
  bit_n(3, cpu->e());
}

void cb_5c(CPU *cpu) {
  //bit 3, h
  bit_n(3, cpu->h());
}

void cb_5d(CPU *cpu) {
  //bit 3, l
  bit_n(3, cpu->l());
}

void cb_5e(CPU *cpu) {
  //bit 3, (hl)
  bit_n(3, cpu->read8(cpu->hl()));
}

void cb_5f(CPU *cpu) {
  //bit 3, a
  bit_n(3, cpu->a());
}

void cb_60(CPU *cpu) {
  //bit 4, b
  bit_n(4, cpu->b());
}

void cb_61(CPU *cpu) {
  //bit 4, c
  bit_n(4, cpu->c());
}

void cb_62(CPU *cpu) {
  //bit 4, d
  bit_n(4, cpu->d());
}

void cb_63(CPU *cpu) {
  //bit 4, e
  bit_n(4, cpu->e());
}

void cb_64(CPU *cpu) {
  //bit 4, h
  bit_n(4, cpu->h());
}

void cb_65(CPU *cpu) {
  //bit 4, l
  bit_n(4, cpu->l());
}

void cb_66(CPU *cpu) {
  //bit 4, (hl)
  bit_n(4, cpu->read8(cpu->hl()));
}

void cb_67(CPU *cpu) {
  //bit 4, a
  bit_n(4, cpu->a());
}

void cb_68(CPU *cpu) {
  //bit 5, b
  bit_n(5, cpu->b());
}

void cb_69(CPU *cpu) {
  //bit 5, c
  bit_n(5, cpu->c());
}

void cb_6a(CPU *cpu) {
  //bit 5, d
  bit_n(5, cpu->d());
}

void cb_6b(CPU *cpu) {
  //bit 5, e
  bit_n(5, cpu->e());
}

void cb_6c(CPU *cpu) {
  //bit 5, h
  bit_n(5, cpu->h());
}

void cb_6d(CPU *cpu) {
  //bit 5, l
  bit_n(5, cpu->l());
}

void cb_6e(CPU *cpu) {
  //bit 5, (hl)
  bit_n(5, cpu->read8(cpu->hl()));
}

void cb_6f(CPU *cpu) {
  //bit 5, a
  bit_n(5, cpu->a());
}

void cb_70(CPU *cpu) {
  //bit 6, b
  bit_n(6, cpu->b());
}

void cb_71(CPU *cpu) {
  //bit 6, c
  bit_n(6, cpu->c());
}

void cb_72(CPU *cpu) {
  //bit 6, d
  bit_n(6, cpu->d());
}

void cb_73(CPU *cpu) {
  //bit 6, e
  bit_n(6, cpu->e());
}

void cb_74(CPU *cpu) {
  //bit 6, h
  bit_n(6, cpu->h());
}

void cb_75(CPU *cpu) {
  //bit 6, l
  bit_n(6, cpu->l());
}

void cb_76(CPU *cpu) {
  //bit 6, (hl)
  bit_n(6, cpu->read8(cpu->hl()));
}

void cb_77(CPU *cpu) {
  //bit 6, a
  bit_n(6, cpu->a());
}

void cb_78(CPU *cpu) {
  //bit 7, b
  bit_n(7, cpu->b());
}

void cb_79(CPU *cpu) {
  //bit 7, c
  bit_n(7, cpu->c());
}

void cb_7a(CPU *cpu) {
  //bit 7, d
  bit_n(7, cpu->d());
}

void cb_7b(CPU *cpu) {
  //bit 7, e
  bit_n(7, cpu->e());
}

void cb_7c(CPU *cpu) {
  //bit 7, h
  bit_n(7, cpu->h());
}

void cb_7d(CPU *cpu) {
  //bit 7, l
  bit_n(7, cpu->l());
}

void cb_7e(CPU *cpu) {
  //bit 7, (hl)
  bit_n(7, cpu->read8(cpu->hl()));
}

void cb_7f(CPU *cpu) {
  //bit 7, a
  bit_n(7, cpu->a());
}

void cb_80(CPU *cpu) {
  //res 0, b
  res_r(0, b);
}

void cb_81(CPU *cpu) {
  //res 0, c
  res_r(0, c);
}

void cb_82(CPU *cpu) {
  //res 0, d
  res_r(0, d);
}

void cb_83(CPU *cpu) {
  //res 0, e
  res_r(0, e);
}

void cb_84(CPU *cpu) {
  //res 0, h
  res_r(0, h);
}

void cb_85(CPU *cpu) {
  //res 0, l
  res_r(0, l);
}

void cb_86(CPU *cpu) {
  //res 0, (hl)
  res_hl(0);
}

void cb_87(CPU *cpu) {
  //res 0, a
  res_r(0, a);
}

void cb_88(CPU *cpu) {
  //res 1, b
  res_r(1, b);
}

void cb_89(CPU *cpu) {
  //res 1, c
  res_r(1, c);
}

void cb_8a(CPU *cpu) {
  //res 1, d
  res_r(1, d);
}

void cb_8b(CPU *cpu) {
  //res 1, e
  res_r(1, e);
}

void cb_8c(CPU *cpu) {
  //res 1, h
  res_r(1, h);
}

void cb_8d(CPU *cpu) {
  //res 1, l
  res_r(1, l);
}

void cb_8e(CPU *cpu) {
  //res 1, (hl)
  res_hl(1);
}

void cb_8f(CPU *cpu) {
  //res 1, a
  res_r(1, a);
}

void cb_90(CPU *cpu) {
  //res 2, b
  res_r(2, b);
}

void cb_91(CPU *cpu) {
  //res 2, c
  res_r(2, c);
}

void cb_92(CPU *cpu) {
  //res 2, d
  res_r(2, d);
}

void cb_93(CPU *cpu) {
  //res 2, e
  res_r(2, e);
}

void cb_94(CPU *cpu) {
  //res 2, h
  res_r(2, h);
}

void cb_95(CPU *cpu) {
  //res 2, l
  res_r(2, l);
}

void cb_96(CPU *cpu) {
  //res 2, (hl)
  res_hl(2);
}

void cb_97(CPU *cpu) {
  //res 2, a
  res_r(2, a);
}

void cb_98(CPU *cpu) {
  //res 3, b
  res_r(3, b);
}

void cb_99(CPU *cpu) {
  //res 3, c
  res_r(3, c);
}

void cb_9a(CPU *cpu) {
  //res 3, d
  res_r(3, d);
}

void cb_9b(CPU *cpu) {
  //res 3, e
  res_r(3, e);
}

void cb_9c(CPU *cpu) {
  //res 3, h
  res_r(3, h);
}

void cb_9d(CPU *cpu) {
  //res 3, l
  res_r(3, l);
}

void cb_9e(CPU *cpu) {
  //res 3, (hl)
  res_hl(3);
}

void cb_9f(CPU *cpu) {
  //res 3, a
  res_r(3, a);
}

void cb_a0(CPU *cpu) {
  //res 4, b
  res_r(4, b);
}

void cb_a1(CPU *cpu) {
  //res 4, c
  res_r(4, c);
}

void cb_a2(CPU *cpu) {
  //res 4, d
  res_r(4, d);
}

void cb_a3(CPU *cpu) {
  //res 4, e
  res_r(4, e);
}

void cb_a4(CPU *cpu) {
  //res 4, h
  res_r(4, h);
}

void cb_a5(CPU *cpu) {
  //res 4, l
  res_r(4, l);
}

void cb_a6(CPU *cpu) {
  //res 4, (hl)
  res_hl(4);
}

void cb_a7(CPU *cpu) {
  //res 4, a
  res_r(4, a);
}

void cb_a8(CPU *cpu) {
  //res 5, b
  res_r(5, b);
}

void cb_a9(CPU *cpu) {
  //res 5, c
  res_r(5, c);
}

void cb_aa(CPU *cpu) {
  //res 5, d
  res_r(5, d);
}

void cb_ab(CPU *cpu) {
  //res 5, e
  res_r(5, e);
}

void cb_ac(CPU *cpu) {
  //res 5, h
  res_r(5, h);
}

void cb_ad(CPU *cpu) {
  //res 5, l
  res_r(5, l);
}

void cb_ae(CPU *cpu) {
  //res 5, (hl)
  res_hl(5);
}

void cb_af(CPU *cpu) {
  //res 5, a
  res_r(5, a);
}

void cb_b0(CPU *cpu) {
  //res 6, b
  res_r(6, b);
}

void cb_b1(CPU *cpu) {
  //res 6, c
  res_r(6, c);
}

void cb_b2(CPU *cpu) {
  //res 6, d
  res_r(6, d);
}

void cb_b3(CPU *cpu) {
  //res 6, e
  res_r(6, e);
}

void cb_b4(CPU *cpu) {
  //res 6, h
  res_r(6, h);
}

void cb_b5(CPU *cpu) {
  //res 6, l
  res_r(6, l);
}

void cb_b6(CPU *cpu) {
  //res 6, (hl)
  res_hl(6);
}

void cb_b7(CPU *cpu) {
  //res 6, a
  res_r(6, a);
}

void cb_b8(CPU *cpu) {
  //res 7, b
  res_r(7, b);
}

void cb_b9(CPU *cpu) {
  //res 7, c
  res_r(7, c);
}

void cb_ba(CPU *cpu) {
  //res 7, d
  res_r(7, d);
}

void cb_bb(CPU *cpu) {
  //res 7, e
  res_r(7, e);
}

void cb_bc(CPU *cpu) {
  //res 7, h
  res_r(7, h);
}

void cb_bd(CPU *cpu) {
  //res 7, l
  res_r(7, l);
}

void cb_be(CPU *cpu) {
  //res 7, (hl)
  res_hl(7);
}

void cb_bf(CPU *cpu) {
  //res 7, a
  res_r(7, a);
}

void cb_c0(CPU *cpu) {
  //set 0, b
  set_r(0, b);
}

void cb_c1(CPU *cpu) {
  //set 0, c
  set_r(0, c);
}

void cb_c2(CPU *cpu) {
  //set 0, d
  set_r(0, d);
}

void cb_c3(CPU *cpu) {
  //set 0, e
  set_r(0, e);
}

void cb_c4(CPU *cpu) {
  //set 0, h
  set_r(0, h);
}

void cb_c5(CPU *cpu) {
  //set 0, l
  set_r(0, l);
}

void cb_c6(CPU *cpu) {
  //set 0, (hl)
  set_hl(0);
}

void cb_c7(CPU *cpu) {
  //set 0, a
  set_r(0, a);
}

void cb_c8(CPU *cpu) {
  //set 1, b
  set_r(1, b);
}

void cb_c9(CPU *cpu) {
  //set 1, c
  set_r(1, c);
}

void cb_ca(CPU *cpu) {
  //set 1, d
  set_r(1, d);
}

void cb_cb(CPU *cpu) {
  //set 1, e
  set_r(1, e);
}

void cb_cc(CPU *cpu) {
  //set 1, h
  set_r(1, h);
}

void cb_cd(CPU *cpu) {
  //set 1, l
  set_r(1, l);
}

void cb_ce(CPU *cpu) {
  //set 1, (hl)
  set_hl(1);
}

void cb_cf(CPU *cpu) {
  //set 1, a
  set_r(1, a);
}

void cb_d0(CPU *cpu) {
  //set 2, b
  set_r(2, b);
}

void cb_d1(CPU *cpu) {
  //set 2, c
  set_r(2, c);
}

void cb_d2(CPU *cpu) {
  //set 2, d
  set_r(2, d);
}

void cb_d3(CPU *cpu) {
  //set 2, e
  set_r(2, e);
}

void cb_d4(CPU *cpu) {
  //set 2, h
  set_r(2, h);
}

void cb_d5(CPU *cpu) {
  //set 2, l
  set_r(2, l);
}

void cb_d6(CPU *cpu) {
  //set 2, (hl)
  set_hl(2);
}

void cb_d7(CPU *cpu) {
  //set 2, a
  set_r(2, a);
}

void cb_d8(CPU *cpu) {
  //set 3, b
  set_r(3, b);
}

void cb_d9(CPU *cpu) {
  //set 3, c
  set_r(3, c);
}

void cb_da(CPU *cpu) {
  //set 3, d
  set_r(3, d);
}

void cb_db(CPU *cpu) {
  //set 3, e
  set_r(3, e);
}

void cb_dc(CPU *cpu) {
  //set 3, h
  set_r(3, h);
}

void cb_dd(CPU *cpu) {
  //set 3, l
  set_r(3, l);
}

void cb_de(CPU *cpu) {
  //set 3, (hl)
  set_hl(3);
}

void cb_df(CPU *cpu) {
  //set 3, a
  set_r(3, a);
}

void cb_e0(CPU *cpu) {
  //set 4, b
  set_r(4, b);
}

void cb_e1(CPU *cpu) {
  //set 4, c
  set_r(4, c);
}

void cb_e2(CPU *cpu) {
  //set 4, d
  set_r(4, d);
}

void cb_e3(CPU *cpu) {
  //set 4, e
  set_r(4, e);
}

void cb_e4(CPU *cpu) {
  //set 4, h
  set_r(4, h);
}

void cb_e5(CPU *cpu) {
  //set 4, l
  set_r(4, l);
}

void cb_e6(CPU *cpu) {
  //set 4, (hl)
  set_hl(4);
}

void cb_e7(CPU *cpu) {
  //set 4, a
  set_r(4, a);
}

void cb_e8(CPU *cpu) {
  //set 5, b
  set_r(5, b);
}

void cb_e9(CPU *cpu) {
  //set 5, c
  set_r(5, c);
}

void cb_ea(CPU *cpu) {
  //set 5, d
  set_r(5, d);
}

void cb_eb(CPU *cpu) {
  //set 5, e
  set_r(5, e);
}

void cb_ec(CPU *cpu) {
  //set 5, h
  set_r(5, h);
}

void cb_ed(CPU *cpu) {
  //set 5, l
  set_r(5, l);
}

void cb_ee(CPU *cpu) {
  //set 5, (hl)
  set_hl(5);
}

void cb_ef(CPU *cpu) {
  //set 5, a
  set_r(5, a);
}

void cb_f0(CPU *cpu) {
  //set 6, b
  set_r(6, b);
}

void cb_f1(CPU *cpu) {
  //set 6, c
  set_r(6, c);
}

void cb_f2(CPU *cpu) {
  //set 6, d
  set_r(6, d);
}

void cb_f3(CPU *cpu) {
  //set 6, e
  set_r(6, e);
}

void cb_f4(CPU *cpu) {
  //set 6, h
  set_r(6, h);
}

void cb_f5(CPU *cpu) {
  //set 6, l
  set_r(6, l);
}

void cb_f6(CPU *cpu) {
  //set 6, (hl)
  set_hl(6);
}

void cb_f7(CPU *cpu) {
  //set 6, a
  set_r(6, a);
}

void cb_f8(CPU *cpu) {
  //set 7, b
  set_r(7, b);
}

void cb_f9(CPU *cpu) {
  //set 7, c
  set_r(7, c);
}

void cb_fa(CPU *cpu) {
  //set 7, d
  set_r(7, d);
}

void cb_fb(CPU *cpu) {
  //set 7, e
  set_r(7, e);
}

void cb_fc(CPU *cpu) {
  //set 7, h
  set_r(7, h);
}

void cb_fd(CPU *cpu) {
  //set 7, l
  set_r(7, l);
}

void cb_fe(CPU *cpu) {
  //set 7, (hl)
  set_hl(7);
}

void cb_ff(CPU *cpu) {
  //set 7, a
  set_r(7, a);
}


void (*cbs[])(CPU *) = {
  cb_00, cb_01, cb_02, cb_03, cb_04, cb_05, cb_06, cb_07, cb_08, cb_09, cb_0a, cb_0b, cb_0c, cb_0d, cb_0e, cb_0f,
  cb_10, cb_11, cb_12, cb_13, cb_14, cb_15, cb_16, cb_17, cb_18, cb_19, cb_1a, cb_1b, cb_1c, cb_1d, cb_1e, cb_1f,
  cb_20, cb_21, cb_22, cb_23, cb_24, cb_25, cb_26, cb_27, cb_28, cb_29, cb_2a, cb_2b, cb_2c, cb_2d, cb_2e, cb_2f,
  cb_30, cb_31, cb_32, cb_33, cb_34, cb_35, cb_36, cb_37, cb_38, cb_39, cb_3a, cb_3b, cb_3c, cb_3d, cb_3e, cb_3f,
  cb_40, cb_41, cb_42, cb_43, cb_44, cb_45, cb_46, cb_47, cb_48, cb_49, cb_4a, cb_4b, cb_4c, cb_4d, cb_4e, cb_4f,
  cb_50, cb_51, cb_52, cb_53, cb_54, cb_55, cb_56, cb_57, cb_58, cb_59, cb_5a, cb_5b, cb_5c, cb_5d, cb_5e, cb_5f,
  cb_60, cb_61, cb_62, cb_63, cb_64, cb_65, cb_66, cb_67, cb_68, cb_69, cb_6a, cb_6b, cb_6c, cb_6d, cb_6e, cb_6f,
  cb_70, cb_71, cb_72, cb_73, cb_74, cb_75, cb_76, cb_77, cb_78, cb_79, cb_7a, cb_7b, cb_7c, cb_7d, cb_7e, cb_7f,
  cb_80, cb_81, cb_82, cb_83, cb_84, cb_85, cb_86, cb_87, cb_88, cb_89, cb_8a, cb_8b, cb_8c, cb_8d, cb_8e, cb_8f,
  cb_90, cb_91, cb_92, cb_93, cb_94, cb_95, cb_96, cb_97, cb_98, cb_99, cb_9a, cb_9b, cb_9c, cb_9d, cb_9e, cb_9f,
  cb_a0, cb_a1, cb_a2, cb_a3, cb_a4, cb_a5, cb_a6, cb_a7, cb_a8, cb_a9, cb_aa, cb_ab, cb_ac, cb_ad, cb_ae, cb_af,
  cb_b0, cb_b1, cb_b2, cb_b3, cb_b4, cb_b5, cb_b6, cb_b7, cb_b8, cb_b9, cb_ba, cb_bb, cb_bc, cb_bd, cb_be, cb_bf,
  cb_c0, cb_c1, cb_c2, cb_c3, cb_c4, cb_c5, cb_c6, cb_c7, cb_c8, cb_c9, cb_ca, cb_cb, cb_cc, cb_cd, cb_ce, cb_cf,
  cb_d0, cb_d1, cb_d2, cb_d3, cb_d4, cb_d5, cb_d6, cb_d7, cb_d8, cb_d9, cb_da, cb_db, cb_dc, cb_dd, cb_de, cb_df,
  cb_e0, cb_e1, cb_e2, cb_e3, cb_e4, cb_e5, cb_e6, cb_e7, cb_e8, cb_e9, cb_ea, cb_eb, cb_ec, cb_ed, cb_ee, cb_ef,
  cb_f0, cb_f1, cb_f2, cb_f3, cb_f4, cb_f5, cb_f6, cb_f7, cb_f8, cb_f9, cb_fa, cb_fb, cb_fc, cb_fd, cb_fe, cb_ff,
};
}
