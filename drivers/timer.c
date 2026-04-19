#include "timer.h"
#include "ports.h"
#include "../cpu/isr.h"
#include "screen.h"
#include "../libc/string.h"
#include "../cpu/tasking/task.h"

uint32_t tick = 0;

// Callback 
static void timer_callback(registers_t *r) {
    tick++;

    // Como o task_switch vai "teleportar" a CPU, o kernel não vai chegar
    // ao final da interrupção para dar o EOI padrão. 
    // É necessário avisar o PIC que o sinal foi recebido
    port_byte_out(0x20, 0x20);

    task_switch();
}

// Ligar o PIT
void init_timer(uint32_t freq) {
    // Cadastrar a função timer_callback para a porta 32 do isr (IRQ 0)
    register_interrupt_handler(32, timer_callback);

    // Divisor para mandar em duas metades
    uint32_t divisor = 1193180 / freq;

    // Enviar o comando para a porta 0x43 (Porta de "configuração" do PIT)
    port_byte_out(0x43, 0x36);

    uint32_t low = (uint8_t)(divisor & 0xFF); // Metade de baixo
    uint32_t high = (uint8_t)((divisor >> 8) & 0xFF); // Metade de cima

    // Enviar os dados para a porta 0x40
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}