import os
from textwrap import dedent

NAME = "interpreter_cb"


def main():
    print("This script is currently disabled, but was used to generate the initial files")
    exit(1)
    # write_header()
    # write_impl()


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
        header.write(f"void op_{opstring}(CPU *cpu);\n")

    header.write(dedent(
        f"""
        void (*ops[])(CPU *) = {{\
        """
    ))
    for op in range(0, 0x100):
        opstring = "%0.2x" % op
        if op % 16 == 0:
            header.write("\n  ")
        header.write(f"op_{opstring},")
        if op % 16 != 15:
            header.write(" ")
    header.write(dedent(
        """
        }
        """
    ))

    header.write(dedent(
        f"""\
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
    
    """))

    impl.write(dedent(
        f"""\
        void op_00(CPU *cpu) {{
          // nop
        }}
        
        """
    ))
    for op in range(1, 0x100):
        opstring = "%0.2x" % op
        impl.write(dedent(
            f"""\
            void op_{opstring}(CPU *cpu) {{
              op_00(cpu);
            }}
            
            """
        ))
    impl.write(dedent(
        """\
        }
        """))
    impl.close()


main()
