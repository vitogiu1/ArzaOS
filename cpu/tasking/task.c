#include "task.h"
#include "../../kernel/memory/heap.h"
#include "../../kernel/memory/paging.h"
#include "../../drivers/screen.h"

// Variáveis globais
volatile task_t *current_task;      // Qual processo está na CPU
volatile task_t *ready_queue;       // O ínicio da fila de espera, qual será o próximo processo a trocar
uint16_t next_pid = 1;

// Acesso ao diretório atual do Kernel
extern page_directory_t *kernel_directory;

// Iniciar a Multitarefa
void init_tasking() {
    // Desligar interrupções temporariamente
    __asm__ volatile("cli");

    // Transformar o fluxo atual (O Kernel) numa tarefa "documetnada"
    current_task = (task_t *)kmalloc(sizeof(task_t));

    current_task->pid = next_pid++;
    current_task->state = TASK_RUNNING;
    current_task->esp = 0; // Será preenchido na primeira troca, assim como ebp, e eip
    current_task->ebp = 0;
    current_task->eip = 0;
    current_task->page_dir = kernel_directory;
    current_task->next = (task_t *)current_task;

    // Inicializa com o Kernel
    ready_queue = current_task;
    
    // Liga as interrupções novamente
    __asm__ volatile("sti");
}

// Criar um novo programa (Thread)
void create_thread(void (*process_function)()) {
    __asm__ volatile("cli");

    task_t *new_task = (task_t*)kmalloc(sizeof(task_t));
    new_task->pid = next_pid++;
    new_task->state = TASK_READY;

    // Liberar ao programa uma stack de 4kb
    uint32_t new_stack = (uint32_t)kmalloc(4096);
    new_task->esp = new_stack + 4096;
    new_task->ebp = new_task->esp;

    new_task->eip = (uint32_t)process_function;
    new_task->page_dir = current_task->page_dir;

    // Insere na lista de processos
    task_t *tmp = (task_t*)current_task;
    while(tmp->next != (task_t *)current_task){
        tmp= tmp->next;
    }
    tmp->next = new_task;
    new_task->next = (task_t *)current_task;

    __asm__ volatile("sti");
}

void task_switch() {
    //Se só existe 1 programa rodando, não faz sentido trocar
    if(!current_task || current_task->next == current_task) return;

    uint32_t eip = read_eip();

    // Se o EIP for 0x12345, significa que a task não foi chamada, mas está retornando
    if(eip == 0x12345) return;

    // Se não for o 0x12345, significa que a task está próxima de seru pausada
    // Guardar a linha onde a task estava e a stack
    current_task->eip = eip;
    __asm__ volatile("mov %%esp, %0" : "=r"(current_task->esp));
    __asm__ volatile("mov %%ebp, %0": "=r"(current_task->ebp));

    current_task = current_task->next;

    switch_task(current_task->eip, current_task->esp, current_task->ebp, (uint32_t)current_task->page_dir);
}