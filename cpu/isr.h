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
void isr_handler(registers_t r);

// 3. Cadastrando os Guarda-Costas do Assembly (As 32 Exceções da CPU)
// (Declaramos apenas os primeiros essenciais por enquanto para manter limpo)
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
extern uint32_t isr31;

#endif