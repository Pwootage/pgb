import os
from textwrap import dedent

NAME = "interpreter_cb"


def main():
    write_header()
    write_impl()


def write_header():
    header = open(f"{NAME}.hpp", "w")
    header.write(dedent(
        f"""\
        #ifndef PGB_{NAME.upper()}_HPP
        #define PGB_{NAME.upper()}_HPP
        #include "../CPU.hpp"
        
        namespace {NAME} {{
        """
    ))
    for op in range(0, 0x100):
        opstring = "%0.2x" % op
        header.write(f"void cb_{opstring}(CPU *cpu);\n")



    header.write(dedent(
        f"""\
        
        extern void (*cbs[])(CPU *);

        }}
        
        #endif //PGB_{NAME.upper()}_HPP
        """
    ))
    header.close()


def write_impl():
    impl = open(f"{NAME}.cpp", "w")
    impl.write(dedent(f"""\
    #include "{NAME}.hpp"
    
    namespace {NAME} {{

    #define bit_n(bit, value) do {{\\
      uint8_t v = (value);\\
      uint8_t mask = 1 << bit;\\
      cpu->zero((v & mask) != 0);\\
      cpu->sub(false);\\
      cpu->halfCarry(1);\\
    }} while(0)

    #define set_r(bit, reg) do {{\\
      uint8_t mask = 1u << bit;\\
      cpu->reg(cpu->reg() | mask);\\
    }} while(0)

    #define set_hl(bit) do {{\\
      uint8_t mask = 1u << bit;\\
      cpu->write8(cpu->hl(), cpu->read8(cpu->hl()) | mask);\\
    }} while(0)

    #define res_r(bit, reg) do {{\\
      uint8_t mask = 1u << bit;\\
      mask = ~mask;\\
      cpu->reg(cpu->reg() & mask);\\
    }} while(0)

    #define res_hl(bit) do {{\\
      uint8_t mask = 1u << bit;\\
      mask = ~mask;\\
      cpu->write8(cpu->hl(), cpu->read8(cpu->hl()) & mask);\\
    }} while(0)

    #define rlc() \\
      uint8_t carry = (v & 0x80) >> 7u;\\
      uint8_t res = (v << 1) | carry;\\
      cpu->zero(res == 0);\\
      cpu->sub(false);\\
      cpu->halfCarry(false);\\
      cpu->carry(carry != 0);

    #define rl() \\
      uint8_t oldCarry = cpu->carry() ? 1u : 0u;\\
      bool carry = (v & 0x80) != 0;\\
      uint8_t res = (v << 1) | oldCarry;\\
      cpu->zero(res == 0);\\
      cpu->sub(false);\\
      cpu->halfCarry(false);\\
      cpu->carry(carry);
    
    #define rrc() \\
      uint8_t carry = (v & 0x1) << 7u;\\
      uint8_t res = (v >> 1) | carry;\\
      cpu->zero(res == 0);\\
      cpu->sub(false);\\
      cpu->halfCarry(false);\\
      cpu->carry(carry != 0);

    #define rr() \\
      uint8_t oldCarry = cpu->carry() ? 0x80u : 0u;\\
      bool carry = (v & 0x1) != 0;\\
      uint8_t res = (v >> 1) | oldCarry;\\
      cpu->zero(res == 0);\\
      cpu->sub(false);\\
      cpu->halfCarry(false);\\
      cpu->carry(carry);

    #define sla() \\
      uint8_t carry = (v & 0x80);\\
      uint8_t res = (v << 1);\\
      cpu->zero(res == 0);\\
      cpu->sub(false);\\
      cpu->halfCarry(false);\\
      cpu->carry(carry != 0);

    #define sra() \\
      uint8_t carry = (v & 0x1);\\
      uint8_t msb = (v & 0x80);\\
      uint8_t res = (v >> 1u) | msb;\\
      cpu->zero(res == 0);\\
      cpu->sub(false);\\
      cpu->halfCarry(false);\\
      cpu->carry(carry != 0);

    #define srl() \\
      uint8_t carry = (v & 0x1);\\
      uint8_t res = (v >> 1u);\\
      cpu->zero(res == 0);\\
      cpu->sub(false);\\
      cpu->halfCarry(false);\\
      cpu->carry(carry != 0);

    #define swap() \\
      uint8_t high = (v & 0xF0);\\
      uint8_t low = (v & 0xF0);\\
      uint8_t res = (high >> 4u) | (low << 4u);\\
      cpu->zero(res == 0);\\
      cpu->sub(false);\\
      cpu->halfCarry(false);\\
      cpu->carry(false);
    
    """))

    rot_table = [
        "rlc", "rrc", "rl", "rr", "sla", "sra", "swap", "srl"
    ]
    reg_table = [
        "b", "c", "d", "e", "h", "l", "(hl)", "a"
    ]

    for op in range(0, 0x100):
        x = (op >> 6) & 0x3
        y = (op >> 3) & 0x7
        z = op & 0x7

        instr = None
        opa = None
        opb = None

        opstring = "%0.2x" % op
        if x == 0:
            # Some sort of rotation
            instr = rot_table[y]
            opb = reg_table[z]
            call = dedent(
                f"""\
                uint8_t v = cpu->{opb}();
                {instr}()
                cpu->{opb}(res);\
                """
            )
            if opb == "(hl)":
                call = dedent(
                    f"""\
                      uint8_t v = cpu->read8(cpu->hl());
                      {instr}()
                      cpu->write8(cpu->hl(), res);\
                    """
                )
            impl.write(dedent(
                f"""\
                void cb_{opstring}(CPU *cpu) {{
                  //{instr} {opb}
                  {call}
                }}
                
                """
            ))
        elif x == 1:
            opa = str(y)
            opb = reg_table[z]
            call = f"bit_n({opa}, cpu->{opb}());"
            if opb == "(hl)":
                call = f"bit_n({opa}, cpu->read8(cpu->hl()));"
            impl.write(dedent(
                f"""\
                void cb_{opstring}(CPU *cpu) {{
                  //bit {opa}, {opb}
                  {call}
                }}
                
                """
            ))
        elif x == 2:
            opa = str(y)
            opb = reg_table[z]
            call = f"res_r({opa}, {opb});"
            if opb == "(hl)":
                call = f"res_hl({opa});"
            impl.write(dedent(
                f"""\
                void cb_{opstring}(CPU *cpu) {{
                  //res {opa}, {opb}
                  {call}
                }}
                
                """
            ))
        else:  # x == 3
            opa = str(y)
            opb = reg_table[z]
            call = f"set_r({opa}, {opb});"
            if opb == "(hl)":
                call = f"set_hl({opa});"
            impl.write(dedent(
                f"""\
                void cb_{opstring}(CPU *cpu) {{
                  //set {opa}, {opb}
                  {call}
                }}
                
                """
            ))

    impl.write(dedent(
        f"""
    void (*cbs[])(CPU *) = {{\
    """
    ))
    for op in range(0, 0x100):
        opstring = "%0.2x" % op
        if op % 16 == 0:
            impl.write("\n  ")
        impl.write(f"cb_{opstring},")
        if op % 16 != 15:
            impl.write(" ")
    impl.write(dedent(
        """
        };
        """
    ))

    impl.write(dedent(
        """\
        }
        """))
    impl.close()


main()
