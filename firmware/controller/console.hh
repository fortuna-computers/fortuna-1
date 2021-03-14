#ifndef CONTROLLER_CONSOLE_HH
#define CONTROLLER_CONSOLE_HH

#include "fortuna1.hh"

class Console {
public:
    Console(Serial& serial, Fortuna1& fortuna_1, Buffer& buffer) : serial_(serial), fortuna1_(fortuna_1), buffer_(buffer) {}
    
    void execute(char cmd);
    
private:
    Serial&   serial_;
    Fortuna1& fortuna1_;
    Buffer&   buffer_;
    
    void print_z80_state(RAM const& ram, Z80 const& z80) const;
};


#endif
