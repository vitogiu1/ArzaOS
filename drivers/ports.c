#include "ports.h"

// Função que envia 1 Byte (nosso data) para uma porta de hardware especifica (port)
void port_byte_out(unsigned short port, unsigned char data) {
    // Utilizaremos aqui o código assembly embutido (Inline Assembly) no C
    // Out é a instrução que joga o dado para fora da CPU
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}