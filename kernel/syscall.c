#include "syscall.h"
#include "../cpu/isr.h"
#include "../drivers/screen.h"

// A função que é ativada quando o utilizador chamar "int 0x80"
static void syscall_handler(registers_t *r) {
    // O C recebe a cópia dos registradores (Struct r)
    // O número da syscall será recebido no EAX
    uint32_t syscall_n = r->eax;

    switch (syscall_n)
    {
    case 4:
        // A syscall POSIX 4: SYS_WRITE
        // No Linux, o EBX é o tipo de saída (1 = Tela), 
        // O ECX é a String, e o EDX é o tamanho.
        print((char *)r->ecx, WHITE_ON_BLACK);
        break;
    case 1:
    print("\n[SYSCALL] Um programa pediu para ser encerrado.\n", YELLOW_ON_BLACK);
    break;
    default:
        print("\n[SYSCALL] ERRO: Chamada de Sistema desconhecida: ", RED_ON_BLACK);
        break;
    }
}

void init_syscalls() {
    // Regista a função no roteador da IDT, na porta 128 (0x80)
    register_interrupt_handler(128, syscall_handler);
    print("Portao de Syscalls (int 0x80) Ativado.\n", 0x0A); // Verde
}