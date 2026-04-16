#include "keyboard.h"
#include "ports.h"
#include "../cpu/isr.h"
#include "screen.h"
#include "../libc/string.h"

// Array Normal (Minúsculas e números)
const char scancode_to_char[] = {
    '?', '?', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '?', '?',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '?', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '
};

// Array Shift (Maiúsculas e Símbolos)
const char scancode_to_char_shift[] = {
    '?', '?', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '?', '?',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '?', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', '?', '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', '?', '?', '?', ' '
};

// Scancodes das Teclas Especiais (Set 1)
#define BACKSPACE 0x0E
#define ENTER 0x1C
#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT 0x36
#define LEFT_SHIFT_RELEASE 0xAA
#define RIGHT_SHIFT_RELEASE 0xB6
#define CAPS_LOCK 0x3A

// Variaveis de estado e buffer
static int shift_pressed = 0;
static int caps_lock = 0;

static char key_buffer[256]; // A palavra que esta guardada
static int buffer_index = 0; // Letras ja digitas

// --- A INTELIGÊNCIA ---
static void keyboard_callback(registers_t *r) {
    unsigned char scancode = port_byte_in(0x60);

    // Tratamento de teclas soltas
    if (scancode == LEFT_SHIFT_RELEASE || scancode == RIGHT_SHIFT_RELEASE) {
        shift_pressed = 0; // O usuário soltou o Shift
        return;
    }

    // Tratamento de ações
    if (scancode == CAPS_LOCK) {
        caps_lock = !caps_lock; // Liga/Desliga como um interruptor
        return;
    } 
    else if (scancode == LEFT_SHIFT || scancode == RIGHT_SHIFT) {
        shift_pressed = 1; // O usuário segurou o Shift
        return;
    }
    else if (scancode == BACKSPACE) {
        if (buffer_index > 0) {
            buffer_index--; // Remove do buffer
            key_buffer[buffer_index] = '\0';
            print_backspace(); // Apaga da tela física
        }
        return;
    }
    else if (scancode == ENTER) {
        // Apenas resposta visual, ainda não se trata de um shell
        print("\n", WHITE_ON_BLACK); // Pula linha
        
        print("Comando recebido: '", YELLOW_ON_BLACK);
        print(key_buffer, YELLOW_ON_BLACK);
        print("'\n", YELLOW_ON_BLACK);
        
        buffer_index = 0;
        key_buffer[0] = '\0'; 
        
        print("> ", WHITE_ON_BLACK);
        return;
    }


    // Tratamento de letras normais
    if (scancode <= 57) {
        char letra = '?';
        if (shift_pressed || caps_lock) {
            letra = scancode_to_char_shift[scancode];
        } else {
            letra = scancode_to_char[scancode];
        }

        if (buffer_index < 255) {
            key_buffer[buffer_index] = letra;
            buffer_index++;
            key_buffer[buffer_index] = '\0';
            char str[2] = {letra, '\0'};
            print(str, WHITE_ON_BLACK);
        }
    }
}

// Inicializador do Driver
void init_keyboard() {
    // Registra a porta 33 para esse driver atual (do keyboard)
    register_interrupt_handler(33, keyboard_callback);
}