#include "serial.hh"

#include <stdio.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

Serial
Serial::init()
{
    // set baud rate - http://ruemohr.org/~ircjunk/avr/baudcalc/avrbaudcalc-1.0.8.php?postbitrate=38400&postclock=8
    // int ubrr = 12;     // 38400 at 8 Mhz
    // int ubrr = 25;     // 38400 at 16 Mhz
    // int ubrr = 8;     // 115200 at 16 Mhz
    int ubrr = 26;       // 38400 at 16.5 Mhz
    UBRR0H = (ubrr>>8);
    UBRR0L = (ubrr);

    // set config
    UCSR0C = (1<<UMSEL01) | (1<<UCSZ01) | (1<<UCSZ00);   // Async-mode 
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);     // Enable Receiver and Transmitter

    // static FILE uart = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
    static FILE uart;
    uart.put = [](char c, FILE*) {
        while (!( UCSR0A & (1<<UDRE0))); // Wait for empty transmit buffer
        UDR0 = c;
        if (c == '\n') {
            while (!( UCSR0A & (1<<UDRE0)));
            UDR0 = '\r';
        }
        return 0;
    };
    uart.get = [](FILE*) -> int {
        while (!( UCSR0A & (1<<RXC0)));  // wait for empty receive buffer
        return UDR0;
    };
    uart.flags = _FDEV_SETUP_RW;
    stdout = stdin = &uart;

    return Serial();
}

void
Serial::send(uint8_t byte) const
{
    while (!( UCSR0A & (1<<UDRE0))); // Wait for empty transmit buffer
    UDR0 = byte;
}

uint8_t
Serial::recv() const
{
    while (!( UCSR0A & (1<<RXC0)));  // wait for empty receive buffer
    return UDR0;
}

uint16_t
Serial::recv16() const
{
    uint16_t a = recv();
    return (a << 8) | recv();
}

uint8_t
Serial::recv_noblock() const
{
    if (UCSR0A & (1<<RXC0))
        return UDR0;
    else
        return 0;
}

void
Serial::clrscr() const
{
    printf_P(PSTR("\e[1;1H\e[2J"));  // clear screen
}