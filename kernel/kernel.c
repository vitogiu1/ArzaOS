// Estamos no modo Freestanding de C, ou seja, funções padrões e bibliotecas de Sistemas Operacionais não estão disponível
// Como por exemplo, o stdio.h. Neste caso, a nossa função de iniciaçização, não é mais obrigatoriamente a main()
#include "../drivers/screen.h"
#include "../libc/string.h"
#include "../cpu/isr.h"
#include "../cpu/idt.h"
#include "../drivers/pic.h"
#include "../drivers/keyboard.h"
#include "./memory.h"
#include "pmm.h"

// Função auxiliar para inicializar todas as portas da CPU de uma vez
void isr_install() {
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
    set_idt_gate(31, (uint32_t)&isr31);
    // Cadastrando Periféricos de Hardware
    set_idt_gate(32, (uint32_t)&irq0); // Relógio
    set_idt_gate(33, (uint32_t)&irq1); // Teclado
    
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

    uint32_t bloco1 = pmm_alloc_frame();
    uint32_t bloco2 = pmm_alloc_frame();

    print("Bloco 1 alocado no endereco RAM: ", WHITE_ON_BLACK);
    char s1[16];
    itoa(bloco1, s1);
    print(s1, YELLOW_ON_BLACK); 
    print("\n", WHITE_ON_BLACK);

    print("Bloco 2 alocado no endereco RAM: ", WHITE_ON_BLACK);
    char s2[16];
    itoa(bloco2, s2);
    print(s2, YELLOW_ON_BLACK); 
    print("\n", WHITE_ON_BLACK);

    init_keyboard();
    print("Driver de Teclado Carregado!\n", WHITE_ON_BLACK);
    print("> ", WHITE_ON_BLACK);


    while(1) {} //Trava o Kernel para ele não executar mais nada
}