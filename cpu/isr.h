#ifndef ISR_H
#define ISR_H

#include <stdint.h>

// 1. A Estrutura de Registradores ("A Foto da CPU")
// Essa struct espelha exatamente a ordem que o Assembly vai empilhar as coisas
typedef struct {
    uint32_t ds;                                     // Seletor do Segmento de Dados
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Empurrados pelo comando 'pusha'
    uint32_t int_no, err_code;                       // Número da Interrupção e Código de Erro
    uint32_t eip, cs, eflags, useresp, ss;           // Empurrados automaticamente pelo processador
} registers_t;

// 2. Assinatura do nosso Despachante Central
void isr_handler(registers_t *r);

// 3. Cadastrando TODAS as 32 Exceções da CPU (Intel)
extern uint32_t isr0;
extern uint32_t isr1;
extern uint32_t isr2;
extern uint32_t isr3;
extern uint32_t isr4;
extern uint32_t isr5;
extern uint32_t isr6;
extern uint32_t isr7;
extern uint32_t isr8;
extern uint32_t isr9;
extern uint32_t isr10;
extern uint32_t isr11;
extern uint32_t isr12;
extern uint32_t isr13;
extern uint32_t isr14;
extern uint32_t isr15;
extern uint32_t isr16;
extern uint32_t isr17;
extern uint32_t isr18;
extern uint32_t isr19;
extern uint32_t isr20;
extern uint32_t isr21;
extern uint32_t isr22;
extern uint32_t isr23;
extern uint32_t isr24;
extern uint32_t isr25;
extern uint32_t isr26;
extern uint32_t isr27;
extern uint32_t isr28;
extern uint32_t isr29;
extern uint32_t isr30;
extern uint32_t isr31;

// Syscalls
extern uint32_t isr128;

// 4. Cadastrando  os 16 Periféricos de Hardware (IRQs do chip PIC)
// As IRQs começam na porta 32 da IDT e vão até a 47
extern uint32_t irq0;  // 32: Relógio (Timer)
extern uint32_t irq1;  // 33: Teclado
extern uint32_t irq2;  // 34: Cascata (Comunicação entre os chips PIC)
extern uint32_t irq3;  // 35: COM2 (Serial)
extern uint32_t irq4;  // 36: COM1 (Serial)
extern uint32_t irq5;  // 37: LPT2 (Paralela/Som)
extern uint32_t irq6;  // 38: Disquete
extern uint32_t irq7;  // 39: LPT1 (Impressora)
extern uint32_t irq8;  // 40: CMOS (Relógio em Tempo Real)
extern uint32_t irq9;  // 41: Periféricos Livres
extern uint32_t irq10; // 42: Periféricos Livres
extern uint32_t irq11; // 43: Periféricos Livres
extern uint32_t irq12; // 44: Mouse (Mouse PS/2)
extern uint32_t irq13; // 45: Coprocessador Matemático
extern uint32_t irq14; // 46: Disco Rígido Primário (HD/ATA)
extern uint32_t irq15; // 47: Disco Rígido Secundário

void isr_init_routing();

// O Despachante de Periféricos
void irq_handler(registers_t *r);

// Define que 'isr_t' é um tipo de variável que guarda uma função
typedef void (*isr_t)(registers_t*);

// Função para os Drivers se cadastrarem no roteador
void register_interrupt_handler(uint8_t n, isr_t handler);

#endif