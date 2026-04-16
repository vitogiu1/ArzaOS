#include "isr.h"
#include "../drivers/screen.h"
#include "../libc/string.h"
#include "../drivers/pic.h"
#include "../drivers/ports.h"

// Array contendo os nomes oficiais das exceções da Intel
char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault", // Porta 13
    "Page Fault",               // Porta 14
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved" 
};

isr_t interrupt_handlers[256];

// Função para varrer a RAM e zerar todas as gavetas do roteador
void isr_init_routing() {
    for (int i = 0; i < 256; i++) {
        interrupt_handlers[i] = 0;
    }
}

// O DESPACHANTE CENTRAL
void isr_handler(registers_t *r) {
    print("\nKERNEL PANIC\n", RED_ON_BLACK);
    print("Excecao da CPU Detectada: ", YELLOW_ON_BLACK);
    
    // Imprime o nome do erro puxando do Array
    print(exception_messages[r->int_no], YELLOW_ON_BLACK);
    
    // Imprime o número da porta que causou o problema
    print("\nPorta de Interrupcao (ID): ", WHITE_ON_BLACK);
    char s[3];
    itoa(r->int_no, s);
    print(s, WHITE_ON_BLACK);
    print("\nO Sistema foi paralisado por seguranca.\n", RED_ON_BLACK);

    // Se a interrupção for menor que 32, é um erro interno da CPU irrecuperável
    if(r->int_no < 32) {
    // Desliga a capacidade da CPU de ouvir novas interrupções e paralisa o chip.
    __asm__ volatile(
        "cli\n\t"
        "hlt"
    );

    while(1);
    }
}

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

// O DESPACHANTE DE HARDWARE
void irq_handler(registers_t *r) {
    // Verifica se existe algum driver cadastrado para a porta que tocou
    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r); // Chama o Driver
    }

    // O Agradecimento (Obrigatório)
    pic_send_eoi(r->int_no - 32);
}