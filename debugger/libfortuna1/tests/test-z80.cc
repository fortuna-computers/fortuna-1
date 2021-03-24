#include <iostream>

#include "tsupport.hh"

int main(int argc, char* argv[])
{
    TestArgs t(argc, argv);
    auto f = t.create_fortuna();
    
    // two NOPs, a JP and a RST
    title("NOPs");
    f->ram_write_buffer(0, { 0x0, 0x0, 0xc3, 0xc3, 0xc3 });  // NOP, NOP, JP C3C3H
    f->ram_write_byte(0xc3c3, 0xff);                          // RST 38H
    f->soft_reset();
    ASSERT_EQ("Initial state: PC == 0", 0, f->z80_info().pc);
    ASSERT_EQ("NOP: PC == 1", 1, f->z80_step().pc);
    ASSERT_EQ("NOP: PC == 2", 2, f->z80_step().pc);
    ASSERT_EQ("JP C3C3H: PC == 0xC3C3", 0xc3c3, f->z80_step().pc);
    ASSERT_EQ("RST 38H: PC == 0x38", 0x38, f->z80_step().pc);

    // two-byte instruction
    title("Two-byte instruction");
    f->ram_write_buffer(0, { 0x6, 0xaf });  // LD B, $AF
    f->soft_reset();
    ASSERT_EQ("Initial state: PC == 0", 0, f->z80_info().pc);
    ASSERT_EQ("After step: PC == 2", 2, f->z80_step().pc);
    
    // compile and execute step
    ASSERT_Q(5, t.run_code(R"(
        ld a, 0x42
        ld (0x8300), a
    )", 2).pc);
    ASSERT_EQ("[0x8300] = 0x42", 0x42, f->ram_read_byte(0x8300));

    // last printed char
    title("Last printed char");
    Z80_Info info = t.run_code(R"(
        ld a, 'H'
        out (0x1), a
        ld a, 'W'
        out (0x1), a)", 1);
    ASSERT_Q(2, info.pc);
    info = f->z80_step();
    ASSERT_Q(4, info.pc);
    std::cout << "\n";
    ASSERT_EQ("Write to string: check last printed char = 'H'", 'H', info.last_printed_char);
    info = f->z80_step();
    ASSERT_Q(6, info.pc);
    ASSERT_EQ("Last printed message was cleared", 0, info.last_printed_char);
    info = f->z80_step();
    ASSERT_Q(8, info.pc);
    ASSERT_EQ("Write to string: check last printed char = 'W'", 'W', info.last_printed_char);
    
    // keypress
    title("Keypress");
    f->ram_write_byte(0x8500, 0);
    t.run_code(R"(
        nop
        in a, (0x1)
        ld (0x8500), a
        nop
    )", 0);
    f->keypress('r');
    ASSERT_Q(0, f->z80_info().pc);
    ASSERT_Q(1, f->z80_step().pc);
    ASSERT_Q(3, f->z80_step().pc);
    ASSERT_Q(6, f->z80_step().pc);
    ASSERT_Q(7, f->z80_step().pc);
    ASSERT_EQ("Receive keypress", 'r', f->ram_read_byte(0x8500));
    
    // keypress interrupt
    title("Keypress interrupt");
    f->ram_write_byte(0x8400, 0);
    t.run_code(R"(
        jp  main
    org 0x8
        in  a, (0x1)
        ld  (0x8400), a
    main:
        im 0
        ei
    cc: jp cc
    )", 6);
    f->keypress('k');
    for (size_t i = 0; i < 6; ++i)
        f->z80_step();
    ASSERT_EQ("Keyboard interrupt was received", 'k', f->ram_read_byte(0x8400));
    
    // extended instructions
    title("Extended instructions");
    info = t.run_code(" nop\n im 0", 2);
    ASSERT_EQ("Step over extended instructions", 3, info.pc);
    info = t.run_code(" nop\n ei", 2);
    ASSERT_EQ("Step over EI", 3, info.pc);
    
    // NMI
    title("NMI");
    t.run_code(" nop\n nop\n nop\n", 2);
    info = f->z80_nmi();
    if (t.real_hardware_mode) {
        ASSERT_EQ("NMI executed", 0x66, info.pc);
    } else {
        ASSERT_EQ("NMI executed", 0x67, info.pc);
    }
}
