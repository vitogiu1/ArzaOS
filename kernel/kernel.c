// Estamos no modo Freestanding de C, ou seja, funções padrões e bibliotecas de Sistemas Operacionais não estão disponível
// Como por exemplo, o stdio.h. Neste caso, a função de inicialização, não é mais obrigatoriamente a main()
#include "../drivers/screen.h"
#include "../libc/string.h"
#include "../cpu/isr.h"
#include "../cpu/idt.h"
#include "../drivers/pic.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "./memory/memory.h"
#include "./memory/pmm.h"
#include "./memory/paging.h"
#include "./memory/heap.h"
#include "../cpu/tasking/task.h"

// Função auxiliar para inicializar todas as portas da CPU de uma vez
void isr_install() {
    // Portas 0 a 31: Exceções Nativas da CPU
    set_idt_gate(0, (uint32_t)&isr0);
    set_idt_gate(1, (uint32_t)&isr1);
    set_idt_gate(2, (uint32_t)&isr2);
    set_idt_gate(3, (uint32_t)&isr3);
    set_idt_gate(4, (uint32_t)&isr4);
    set_idt_gate(5, (uint32_t)&isr5);
    set_idt_gate(6, (uint32_t)&isr6);
    set_idt_gate(7, (uint32_t)&isr7);
    set_idt_gate(8, (uint32_t)&isr8);
    set_idt_gate(9, (uint32_t)&isr9);
    set_idt_gate(10, (uint32_t)&isr10);
    set_idt_gate(11, (uint32_t)&isr11);
    set_idt_gate(12, (uint32_t)&isr12);
    set_idt_gate(13, (uint32_t)&isr13);
    set_idt_gate(14, (uint32_t)&isr14);
    set_idt_gate(15, (uint32_t)&isr15);
    set_idt_gate(16, (uint32_t)&isr16);
    set_idt_gate(17, (uint32_t)&isr17);
    set_idt_gate(18, (uint32_t)&isr18);
    set_idt_gate(19, (uint32_t)&isr19);
    set_idt_gate(20, (uint32_t)&isr20);
    set_idt_gate(21, (uint32_t)&isr21);
    set_idt_gate(22, (uint32_t)&isr22);
    set_idt_gate(23, (uint32_t)&isr23);
    set_idt_gate(24, (uint32_t)&isr24);
    set_idt_gate(25, (uint32_t)&isr25);
    set_idt_gate(26, (uint32_t)&isr26);
    set_idt_gate(27, (uint32_t)&isr27);
    set_idt_gate(28, (uint32_t)&isr28);
    set_idt_gate(29, (uint32_t)&isr29);
    set_idt_gate(30, (uint32_t)&isr30);
    set_idt_gate(31, (uint32_t)&isr31);

    // Portas 32 a 47: Periféricos de Hardware (IRQs)
    set_idt_gate(32, (uint32_t)&irq0); 
    set_idt_gate(33, (uint32_t)&irq1); 
    set_idt_gate(34, (uint32_t)&irq2); 
    set_idt_gate(35, (uint32_t)&irq3); 
    set_idt_gate(36, (uint32_t)&irq4); 
    set_idt_gate(37, (uint32_t)&irq5); 
    set_idt_gate(38, (uint32_t)&irq6); 
    set_idt_gate(39, (uint32_t)&irq7); 
    set_idt_gate(40, (uint32_t)&irq8); 
    set_idt_gate(41, (uint32_t)&irq9); 
    set_idt_gate(42, (uint32_t)&irq10); 
    set_idt_gate(43, (uint32_t)&irq11); 
    set_idt_gate(44, (uint32_t)&irq12); 
    set_idt_gate(45, (uint32_t)&irq13); 
    set_idt_gate(46, (uint32_t)&irq14); 
    set_idt_gate(47, (uint32_t)&irq15); 
    
    set_idt(); // Carrega a tabela na CPU
}

// "Banco de Dados" de estado do Kernel
int service_protected_mode = 1; // Ativado no boot da 0x9000
int service_keyboard = 1;       // Implementado
int service_alert = 0;         // Status de um alerta desligado

void check_service(char* service_name, int status) {
    print("Servico: ", WHITE_ON_BLACK);
    print(service_name, WHITE_ON_BLACK);
    print("... ", WHITE_ON_BLACK);

    if (status == 1) {
        print("[ ATIVO ]\n", GREEN_ON_BLACK);
    } else {
        print("[ INATIVO ]\n", RED_ON_BLACK);
    }
}

void kernel_main() {
    // Ao inicializar o Kernel, a primeira coisa que ele fará, é limpar o que a BIOS deixou na tela
    clear_screen();

    print("ArzaOS v0.1 - Monitor de Kernel\n", WHITE_ON_BLACK);
    print("===============================\n\n", WHITE_ON_BLACK);

    // Verificação dinâmica de serviços
    check_service("Modo Protegido", service_protected_mode);
    check_service("Driver Teclado ", service_keyboard);
    
    if (service_alert) {
        char buffer[10];
        itoa(404, buffer); // Exemplo: Código de erro ou ID do alerta
        print("\nALERTA ATIVO! Codigo: ", YELLOW_ON_BLACK);
        print(buffer, YELLOW_ON_BLACK);
        print("\n", WHITE_ON_BLACK);
    }

    isr_install(); // Instala todas as proteções
    print("Todas as Excecoes da CPU registradas.\n", GREEN_ON_BLACK);

    // Limpa a sujeira da RAM
    isr_init_routing();
    // Tira os periféricos da zona de perigo
    // Mapeia o Master para a Porta 32 e o Slave para a 40
    pic_remap(0x20, 0x28);
    print("Chip PIC Remapeado com sucesso.\n", GREEN_ON_BLACK);

    __asm__ volatile("sti");
    print("Interrupcoes de Hardware ATIVADAS.\n", GREEN_ON_BLACK);

    read_memory_map();

    pmm_init();

    vmm_init();

    init_kernel_heap();

    init_timer(100);
    init_tasking();

    print("Multitarefa iniciado\n", GREEN_ON_BLACK);

    init_keyboard();
    print("Driver de Teclado Carregado!\n", WHITE_ON_BLACK);
    print("> ", WHITE_ON_BLACK);

    while(1) {} //Trava o Kernel para ele não executar mais nada
}