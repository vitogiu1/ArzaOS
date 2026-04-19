#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "../../kernel/memory/paging.h"

// Os possíveis estados de vida de um processo
typedef enum {
    TASK_RUNNING,
    TASK_READY,
    TASK_SLEEPING,
    TASK_DEAD // disponivel para o kfree
} task_state_t;

// A Estrutura do Processo (Task COntrol Block- TCB)
typedef struct task{
    uint32_t pid;           // Process ID
    task_state_t state;     // O estado atual do processo


    // Onde a pilha deste processo parou
    uint32_t esp;           // Stack Pointer
    uint32_t ebp;           // Base Pointer
    uint32_t eip;           // Instruction Pointer (A próxima linha de código)

    page_directory_t *page_dir; // O diretório de páginas (Onde ele está na memória virtual)

    struct task *next;          // Terminando de definir a lista ligada
} task_t;

// Inicializa o sistema de multitarefas
void init_tasking();

void create_thread(void (*process_function)());
void task_switch();

// Funções vindas do assembly
extern uint32_t read_eip();
extern void switch_task(uint32_t eip, uint32_t esp, uint32_t ebp, uint32_t page_directory);

#endif
