#include "isr.h"
#include "../drivers/screen.h"
#include "../libc/string.h"

// Array contendo os nomes oficiais das exceções da Intel!
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
    "Reserved" // Até a porta 31
};

// O DESPACHANTE CENTRAL
void isr_handler(registers_t r) {
    print("\n[!!!] KERNEL PANIC [!!!]\n", RED_ON_BLACK);
    print("Excecao da CPU Detectada: ", YELLOW_ON_BLACK);
    
    // Imprime o nome do erro puxando do Array
    print(exception_messages[r.int_no], YELLOW_ON_BLACK);
    
    // Imprime o número da porta que causou o problema
    print("\nPorta de Interrupcao (ID): ", WHITE_ON_BLACK);
    char s[3];
    itoa(r.int_no, s);
    print(s, WHITE_ON_BLACK);
    print("\nO Sistema foi paralisado por seguranca.\n", RED_ON_BLACK);

    // Se a interrupção for menor que 32, é um erro interno da CPU irrecuperável
    if(r.int_no < 32) {
    // Desliga a capacidade da CPU de ouvir novas interrupções e paralisa o chip.
    __asm__ volatile(
        "cli\n\t"
        "hlt"
    );

    while(1);
    }
}