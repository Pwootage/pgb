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
  cpu->addSub(false);\
  cpu->halfCarry(halfRes > 0xF0u);\
} while (0)

#define dec_r(reg) do {\
  uint8_t r = cpu->reg();\
  uint8_t res = r - 1;\
  uint8_t halfRes = (r & 0x0Fu) - 1;\
  cpu->b(res);\
  \
  cpu->zero(res == 0);\
  cpu->addSub(false);\
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
  cpu->addSub(false);\
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

void op_00(CPU *cpu) {
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
  cpu->addSub(false);
  cpu->halfCarry(false);
  cpu->carry((a & 0x80u) != 0);

  cpu->a(a << 1u);
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
  cpu->addSub(false);
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
  cpu->addSub(false);
  cpu->halfCarry(false);
  cpu->carry((a & 0x80u) != 0);

  cpu->a((a << 1u) | carry);
}

void op_18(CPU *cpu) {
  // jr r8
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
  // ld d, n
  cpu->d(cpu->pcRead8());
}

void op_1f(CPU *cpu) {
  op_00(cpu);
}

void op_20(CPU *cpu) {
  op_00(cpu);
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
  op_00(cpu);
}

void op_28(CPU *cpu) {
  op_00(cpu);
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
  op_00(cpu);
}

void op_30(CPU *cpu) {
  op_00(cpu);
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
  cpu->addSub(false);
  cpu->halfCarry(halfRes > 0xF0u);
}

void op_35(CPU *cpu) {
  // dec (hl)
  uint8_t r = cpu->read8(cpu->hl());
  uint8_t res = r - 1;
  uint8_t halfRes = (r & 0x0Fu) - 1;
  cpu->write8(cpu->hl(), res);

  cpu->zero(res == 0);
  cpu->addSub(false);
  cpu->halfCarry(halfRes > 0xF0u);
}

void op_36(CPU *cpu) {
  // ld (hl), n
  uint8_t imm = cpu->pcRead8();
  cpu->write8(cpu->hl(), imm);
}

void op_37(CPU *cpu) {
  op_00(cpu);
}

void op_38(CPU *cpu) {
  op_00(cpu);
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
  op_00(cpu);
}

void op_40(CPU *cpu) {
  op_00(cpu);
}

void op_41(CPU *cpu) {
  op_00(cpu);
}

void op_42(CPU *cpu) {
  op_00(cpu);
}

void op_43(CPU *cpu) {
  op_00(cpu);
}

void op_44(CPU *cpu) {
  op_00(cpu);
}

void op_45(CPU *cpu) {
  op_00(cpu);
}

void op_46(CPU *cpu) {
  op_00(cpu);
}

void op_47(CPU *cpu) {
  op_00(cpu);
}

void op_48(CPU *cpu) {
  op_00(cpu);
}

void op_49(CPU *cpu) {
  op_00(cpu);
}

void op_4a(CPU *cpu) {
  op_00(cpu);
}

void op_4b(CPU *cpu) {
  op_00(cpu);
}

void op_4c(CPU *cpu) {
  op_00(cpu);
}

void op_4d(CPU *cpu) {
  op_00(cpu);
}

void op_4e(CPU *cpu) {
  op_00(cpu);
}

void op_4f(CPU *cpu) {
  op_00(cpu);
}

void op_50(CPU *cpu) {
  op_00(cpu);
}

void op_51(CPU *cpu) {
  op_00(cpu);
}

void op_52(CPU *cpu) {
  op_00(cpu);
}

void op_53(CPU *cpu) {
  op_00(cpu);
}

void op_54(CPU *cpu) {
  op_00(cpu);
}

void op_55(CPU *cpu) {
  op_00(cpu);
}

void op_56(CPU *cpu) {
  op_00(cpu);
}

void op_57(CPU *cpu) {
  op_00(cpu);
}

void op_58(CPU *cpu) {
  op_00(cpu);
}

void op_59(CPU *cpu) {
  op_00(cpu);
}

void op_5a(CPU *cpu) {
  op_00(cpu);
}

void op_5b(CPU *cpu) {
  op_00(cpu);
}

void op_5c(CPU *cpu) {
  op_00(cpu);
}

void op_5d(CPU *cpu) {
  op_00(cpu);
}

void op_5e(CPU *cpu) {
  op_00(cpu);
}

void op_5f(CPU *cpu) {
  op_00(cpu);
}

void op_60(CPU *cpu) {
  op_00(cpu);
}

void op_61(CPU *cpu) {
  op_00(cpu);
}

void op_62(CPU *cpu) {
  op_00(cpu);
}

void op_63(CPU *cpu) {
  op_00(cpu);
}

void op_64(CPU *cpu) {
  op_00(cpu);
}

void op_65(CPU *cpu) {
  op_00(cpu);
}

void op_66(CPU *cpu) {
  op_00(cpu);
}

void op_67(CPU *cpu) {
  op_00(cpu);
}

void op_68(CPU *cpu) {
  op_00(cpu);
}

void op_69(CPU *cpu) {
  op_00(cpu);
}

void op_6a(CPU *cpu) {
  op_00(cpu);
}

void op_6b(CPU *cpu) {
  op_00(cpu);
}

void op_6c(CPU *cpu) {
  op_00(cpu);
}

void op_6d(CPU *cpu) {
  op_00(cpu);
}

void op_6e(CPU *cpu) {
  op_00(cpu);
}

void op_6f(CPU *cpu) {
  op_00(cpu);
}

void op_70(CPU *cpu) {
  op_00(cpu);
}

void op_71(CPU *cpu) {
  op_00(cpu);
}

void op_72(CPU *cpu) {
  op_00(cpu);
}

void op_73(CPU *cpu) {
  op_00(cpu);
}

void op_74(CPU *cpu) {
  op_00(cpu);
}

void op_75(CPU *cpu) {
  op_00(cpu);
}

void op_76(CPU *cpu) {
  op_00(cpu);
}

void op_77(CPU *cpu) {
  op_00(cpu);
}

void op_78(CPU *cpu) {
  op_00(cpu);
}

void op_79(CPU *cpu) {
  op_00(cpu);
}

void op_7a(CPU *cpu) {
  op_00(cpu);
}

void op_7b(CPU *cpu) {
  op_00(cpu);
}

void op_7c(CPU *cpu) {
  op_00(cpu);
}

void op_7d(CPU *cpu) {
  op_00(cpu);
}

void op_7e(CPU *cpu) {
  op_00(cpu);
}

void op_7f(CPU *cpu) {
  op_00(cpu);
}

void op_80(CPU *cpu) {
  op_00(cpu);
}

void op_81(CPU *cpu) {
  op_00(cpu);
}

void op_82(CPU *cpu) {
  op_00(cpu);
}

void op_83(CPU *cpu) {
  op_00(cpu);
}

void op_84(CPU *cpu) {
  op_00(cpu);
}

void op_85(CPU *cpu) {
  op_00(cpu);
}

void op_86(CPU *cpu) {
  op_00(cpu);
}

void op_87(CPU *cpu) {
  op_00(cpu);
}

void op_88(CPU *cpu) {
  op_00(cpu);
}

void op_89(CPU *cpu) {
  op_00(cpu);
}

void op_8a(CPU *cpu) {
  op_00(cpu);
}

void op_8b(CPU *cpu) {
  op_00(cpu);
}

void op_8c(CPU *cpu) {
  op_00(cpu);
}

void op_8d(CPU *cpu) {
  op_00(cpu);
}

void op_8e(CPU *cpu) {
  op_00(cpu);
}

void op_8f(CPU *cpu) {
  op_00(cpu);
}

void op_90(CPU *cpu) {
  op_00(cpu);
}

void op_91(CPU *cpu) {
  op_00(cpu);
}

void op_92(CPU *cpu) {
  op_00(cpu);
}

void op_93(CPU *cpu) {
  op_00(cpu);
}

void op_94(CPU *cpu) {
  op_00(cpu);
}

void op_95(CPU *cpu) {
  op_00(cpu);
}

void op_96(CPU *cpu) {
  op_00(cpu);
}

void op_97(CPU *cpu) {
  op_00(cpu);
}

void op_98(CPU *cpu) {
  op_00(cpu);
}

void op_99(CPU *cpu) {
  op_00(cpu);
}

void op_9a(CPU *cpu) {
  op_00(cpu);
}

void op_9b(CPU *cpu) {
  op_00(cpu);
}

void op_9c(CPU *cpu) {
  op_00(cpu);
}

void op_9d(CPU *cpu) {
  op_00(cpu);
}

void op_9e(CPU *cpu) {
  op_00(cpu);
}

void op_9f(CPU *cpu) {
  op_00(cpu);
}

void op_a0(CPU *cpu) {
  op_00(cpu);
}

void op_a1(CPU *cpu) {
  op_00(cpu);
}

void op_a2(CPU *cpu) {
  op_00(cpu);
}

void op_a3(CPU *cpu) {
  op_00(cpu);
}

void op_a4(CPU *cpu) {
  op_00(cpu);
}

void op_a5(CPU *cpu) {
  op_00(cpu);
}

void op_a6(CPU *cpu) {
  op_00(cpu);
}

void op_a7(CPU *cpu) {
  op_00(cpu);
}

void op_a8(CPU *cpu) {
  op_00(cpu);
}

void op_a9(CPU *cpu) {
  op_00(cpu);
}

void op_aa(CPU *cpu) {
  op_00(cpu);
}

void op_ab(CPU *cpu) {
  op_00(cpu);
}

void op_ac(CPU *cpu) {
  op_00(cpu);
}

void op_ad(CPU *cpu) {
  op_00(cpu);
}

void op_ae(CPU *cpu) {
  op_00(cpu);
}

void op_af(CPU *cpu) {
  op_00(cpu);
}

void op_b0(CPU *cpu) {
  op_00(cpu);
}

void op_b1(CPU *cpu) {
  op_00(cpu);
}

void op_b2(CPU *cpu) {
  op_00(cpu);
}

void op_b3(CPU *cpu) {
  op_00(cpu);
}

void op_b4(CPU *cpu) {
  op_00(cpu);
}

void op_b5(CPU *cpu) {
  op_00(cpu);
}

void op_b6(CPU *cpu) {
  op_00(cpu);
}

void op_b7(CPU *cpu) {
  op_00(cpu);
}

void op_b8(CPU *cpu) {
  op_00(cpu);
}

void op_b9(CPU *cpu) {
  op_00(cpu);
}

void op_ba(CPU *cpu) {
  op_00(cpu);
}

void op_bb(CPU *cpu) {
  op_00(cpu);
}

void op_bc(CPU *cpu) {
  op_00(cpu);
}

void op_bd(CPU *cpu) {
  op_00(cpu);
}

void op_be(CPU *cpu) {
  op_00(cpu);
}

void op_bf(CPU *cpu) {
  op_00(cpu);
}

void op_c0(CPU *cpu) {
  op_00(cpu);
}

void op_c1(CPU *cpu) {
  op_00(cpu);
}

void op_c2(CPU *cpu) {
  op_00(cpu);
}

void op_c3(CPU *cpu) {
  // jp a16
  uint16_t newPC = cpu->pcRead16();
  jump(newPC);
}

void op_c4(CPU *cpu) {
  op_00(cpu);
}

void op_c5(CPU *cpu) {
  op_00(cpu);
}

void op_c6(CPU *cpu) {
  op_00(cpu);
}

void op_c7(CPU *cpu) {
  op_00(cpu);
}

void op_c8(CPU *cpu) {
  op_00(cpu);
}

void op_c9(CPU *cpu) {
  op_00(cpu);
}

void op_ca(CPU *cpu) {
  op_00(cpu);
}

void op_cb(CPU *cpu) {
  op_00(cpu);
}

void op_cc(CPU *cpu) {
  op_00(cpu);
}

void op_cd(CPU *cpu) {
  op_00(cpu);
}

void op_ce(CPU *cpu) {
  op_00(cpu);
}

void op_cf(CPU *cpu) {
  op_00(cpu);
}

void op_d0(CPU *cpu) {
  op_00(cpu);
}

void op_d1(CPU *cpu) {
  op_00(cpu);
}

void op_d2(CPU *cpu) {
  op_00(cpu);
}

void op_d3(CPU *cpu) {
  op_00(cpu);
}

void op_d4(CPU *cpu) {
  op_00(cpu);
}

void op_d5(CPU *cpu) {
  op_00(cpu);
}

void op_d6(CPU *cpu) {
  op_00(cpu);
}

void op_d7(CPU *cpu) {
  op_00(cpu);
}

void op_d8(CPU *cpu) {
  op_00(cpu);
}

void op_d9(CPU *cpu) {
  op_00(cpu);
}

void op_da(CPU *cpu) {
  op_00(cpu);
}

void op_db(CPU *cpu) {
  op_00(cpu);
}

void op_dc(CPU *cpu) {
  op_00(cpu);
}

void op_dd(CPU *cpu) {
  op_00(cpu);
}

void op_de(CPU *cpu) {
  op_00(cpu);
}

void op_df(CPU *cpu) {
  op_00(cpu);
}

void op_e0(CPU *cpu) {
  op_00(cpu);
}

void op_e1(CPU *cpu) {
  op_00(cpu);
}

void op_e2(CPU *cpu) {
  op_00(cpu);
}

void op_e3(CPU *cpu) {
  op_00(cpu);
}

void op_e4(CPU *cpu) {
  op_00(cpu);
}

void op_e5(CPU *cpu) {
  op_00(cpu);
}

void op_e6(CPU *cpu) {
  op_00(cpu);
}

void op_e7(CPU *cpu) {
  op_00(cpu);
}

void op_e8(CPU *cpu) {
  op_00(cpu);
}

void op_e9(CPU *cpu) {
  op_00(cpu);
}

void op_ea(CPU *cpu) {
  op_00(cpu);
}

void op_eb(CPU *cpu) {
  op_00(cpu);
}

void op_ec(CPU *cpu) {
  op_00(cpu);
}

void op_ed(CPU *cpu) {
  op_00(cpu);
}

void op_ee(CPU *cpu) {
  op_00(cpu);
}

void op_ef(CPU *cpu) {
  op_00(cpu);
}

void op_f0(CPU *cpu) {
  op_00(cpu);
}

void op_f1(CPU *cpu) {
  op_00(cpu);
}

void op_f2(CPU *cpu) {
  op_00(cpu);
}

void op_f3(CPU *cpu) {
  op_00(cpu);
}

void op_f4(CPU *cpu) {
  op_00(cpu);
}

void op_f5(CPU *cpu) {
  op_00(cpu);
}

void op_f6(CPU *cpu) {
  op_00(cpu);
}

void op_f7(CPU *cpu) {
  op_00(cpu);
}

void op_f8(CPU *cpu) {
  op_00(cpu);
}

void op_f9(CPU *cpu) {
  op_00(cpu);
}

void op_fa(CPU *cpu) {
  op_00(cpu);
}

void op_fb(CPU *cpu) {
  op_00(cpu);
}

void op_fc(CPU *cpu) {
  op_00(cpu);
}

void op_fd(CPU *cpu) {
  op_00(cpu);
}

void op_fe(CPU *cpu) {
  op_00(cpu);
}

void op_ff(CPU *cpu) {
  op_00(cpu);
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
