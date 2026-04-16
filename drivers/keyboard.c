#include "keyboard.h"
#include "ports.h"
#include "../cpu/isr.h"
#include "screen.h"
#include "../libc/string.h"

// Mapeamento US-QWERTY - Scancode Set 1
const char scancode_to_char[] = {
    '?', '?', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '?', '?',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '?', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '
};

// Quando a porta 33 trocar, essa função deve ser chamada
static void keyboard_callback(registers_t *r) {
    // Esvaziamos a porta 0x60, lendo-a.
    unsigned char scancode = port_byte_in(0x60);
    
    // Scancodes maiores que 128 significam que a tecla foi SOLTA.
    // É necessário imprimir na tela apenas quando a tecla for PRESSIONADA (< 128).
    if (scancode <= 57) {
        char letra = scancode_to_char[scancode];
        char str[2] = {letra, '\0'};
        print(str, WHITE_ON_BLACK);
    }
}

// Inicializador do Driver
void init_keyboard() {
    // Registra a porta 33 para esse driver atual (do keyboard)
    register_interrupt_handler(33, keyboard_callback);
}