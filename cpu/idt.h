#ifndef IDT_H
#define IDT_H

#include <stdint.h> // Importa os tipos numéricos com tamanho exato (uint32_t, uint16_t, etc)

//  A estrutura de uma IDT Gate (Quem vai escutar quando ouver uma interrupt da placa mãe (não mais da BIOS, como era no bootloader))
typedef struct {
    uint16_t low_offset; // Os 16 bits mais baixos do endereço da função
    uint16_t sel;        // Seletor de Segmento do Kernel (O nosso antigo CODE_SEG)
    uint8_t  always0;    // A Intel exige que este byte seja sempre zero
    uint8_t  flags;      // Flags (Quem pode chamar isso? Qual o tipo de porta?)
    uint16_t high_offset;// Os 16 bits mais altos do endereço da função
} __attribute__((packed)) idt_gate_t;

// (IDT Register)
// É um ponteiro especial que diz à CPU onde a tabela inteira começa e qual o tamanho dela
typedef struct {
    uint16_t limit; // Tamanho da tabela em bytes (- 1)
    uint32_t base;  // Endereço de memória onde a tabela começa
} __attribute__((packed)) idt_register_t;

#define IDT_ENTRIES 256 // A tabela tem exatas 256 posições
extern idt_gate_t idt[IDT_ENTRIES];
extern idt_register_t idt_reg;

// Funções para manipular a IDT
void set_idt_gate(int n, uint32_t handler);
void set_idt();

#endif