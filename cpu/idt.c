#include "idt.h"

// Cria as variáveis físicas na memória
idt_gate_t idt[IDT_ENTRIES];
idt_register_t idt_reg;

// Função para registrar novas interrupts
// n = Número da porta 
// handler =  Endereço da função que vai rodar quando o hardware apitar a interrupt
void set_idt_gate(int n, uint32_t handler) {
    idt[n].low_offset = handler & 0xFFFF;
    idt[n].sel = 0x08; // Garanta que o seu Kernel CS é realmente 0x08 no bootloader!
    idt[n].always0 = 0;
    idt[n].flags = 0x8E; 
    idt[n].high_offset = (handler >> 16) & 0xFFFF;
}

void set_idt() {
    idt_reg.base = (uint32_t) &idt;
    idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;

    // Use o 'lidt' simples, às vezes o 'lidtl' confunde compiladores mais novos.
    __asm__ volatile("lidt (%0)" : : "r" (&idt_reg));
}