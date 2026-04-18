#include "isr.h"
#include "../drivers/screen.h"
#include "../libc/string.h"
#include "../drivers/pic.h"
#include "../drivers/ports.h"

// Array contendo os nomes oficiais das exceções da Intel
char *exception_messages[] = {
    "Division By Zero", "Debug", "Non Maskable Interrupt", "Breakpoint",
    "Into Detected Overflow", "Out of Bounds", "Invalid Opcode", "No Coprocessor",
    "Double Fault", "Coprocessor Segment Overrun", "Bad TSS", "Segment Not Present",
    "Stack Fault", "General Protection Fault", "Page Fault", "Unknown Interrupt",
    "Coprocessor Fault", "Alignment Check", "Machine Check", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved" 
};

// O Catálogo de Tratadores
isr_t interrupt_handlers[256];

// Função para varrer a RAM e zerar todas as gavetas do roteador
void isr_init_routing() {
    for (int i = 0; i < 256; i++) {
        interrupt_handlers[i] = 0;
    }
}

// Função para plugar um motorista/tratador numa porta específica
void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

// O DESPACHANTE CENTRAL (EXCEÇÕES DA CPU)
void isr_handler(registers_t *r) {
    // O SO cadastrou alguma função para tentar salvar esta exceção?
    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r); // Tenta resolver o problema
        return; 
    }

    // Kernel Panic caso ninguém trate
    print("\nKERNEL PANIC\n", RED_ON_BLACK);
    print("Excecao da CPU Detectada: ", YELLOW_ON_BLACK);
    print(exception_messages[r->int_no], YELLOW_ON_BLACK);
    
    print("\nPorta de Interrupcao (ID): ", WHITE_ON_BLACK);
    char s[3];
    itoa(r->int_no, s);
    print(s, WHITE_ON_BLACK);

    // O nosso Detetive de Page Fault embutido
    if(r->int_no == 14) {
        uint32_t fail_address;
        __asm__ __volatile__("mov %%cr2, %0" : "=r"(fail_address));

        print("\nEndereco que causou o Page Fault (CR2): ", RED_WHITE); 
        char s_cr2[16];
        itoa(fail_address, s_cr2);
        print(s_cr2, YELLOW_ON_BLACK);
    }

    print("\nO Sistema foi paralisado por seguranca.\n", RED_ON_BLACK);

    // Trava a máquina de forma absoluta
    __asm__ volatile(
        "cli\n\t"
        "hlt"
    );
    while(1);
}


// O DESPACHANTE DE HARDWARE (IRQs)
void irq_handler(registers_t *r) {
    // Verifica se existe algum driver cadastrado para a porta que tocou (ex: Teclado)
    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r); // Chama o Driver
    }

    // O fallback para o chip PIC
    pic_send_eoi(r->int_no - 32);
}