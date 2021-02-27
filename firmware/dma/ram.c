#include "ram.h"

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <util/delay.h>

uint8_t buffer[512] = {0};

#define DDR_RAM  DDRB
#define PORT_RAM PORTB
#define MREQ     PINB0
#define WE       PINB1
#define RD       PINB2

void ram_init()
{
    DDR_RAM |= (1 << MREQ) | (1 << WE) | (1 << RD);   // set MREQ, WE and RD as outputs
    PORT_RAM |= (1 << MREQ) | (1 << WE) | (1 << RD);  // deactivate them (set to 1)
}

void ram_reset()
{
    // set pins as high impedance
    DDRB &= ~(1 << PINB3);  // A9
    DDRD &= ~0b11111101;    // A8, A10..15
    DDRA = 0x0;             // A0..A7
    DDRC = 0x0;             // D
}

void ram_set_addr(uint16_t addr)
{
    // set address pins as output
    DDRB |= (1 << PINB3);  // A9
    DDRD |= 0b11111101;    // A8, A10..15
    DDRA = 0xff;           // A0..A7

    // set address pins
    PORTA = addr & 0xff;         // A0..A7
    PORTD &= ~0b11111101;        // A8, A10..15
    PORTD |= (addr >> 8) & 0b11111101;
    if (addr & (1 << 8))
        PORTB |= (1 << PINB3);
    else
        PORTB &= ~(1 << PINB3);
}

uint16_t ram_get_addr()
{
    return PORTC;
}

void ram_set_data(uint8_t data)
{
    DDRC = 0xff;
    PORTC = data;
}

uint8_t ram_get_data()
{
    return PINC;
}

void 
ram_write_byte(uint16_t addr, uint8_t data)
{
    ram_set_addr(addr);
    ram_set_data(data);
    PORT_RAM &= ~(1 << MREQ);
    PORT_RAM &= ~(1 << WE);
    _NOP();
    PORT_RAM |= (1 << WE) | (1 << MREQ);
    PORTC = 0;
    ram_reset();
}

uint8_t ram_read_byte(uint16_t addr)
{
    ram_set_addr(addr);
    DDRC = 0;
    PORT_RAM &= ~(1 << MREQ);
    PORT_RAM &= ~(1 << RD);
    _NOP();
    volatile uint8_t data = ram_get_data();
    PORT_RAM |= (1 << RD) | (1 << MREQ);
    ram_reset();
    return data;
}

void
ram_write_buffer(uint16_t addr, uint16_t sz)
{
    (void) addr; (void) sz;
}

void
ram_read_buffer(uint16_t addr, uint16_t sz)
{
    (void) addr; (void) sz;
}
