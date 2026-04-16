#include "screen.h"         // O nosso próprio contrato
#include "ports.h"          // Para usar o port_byte_out
#include "../libc/mem.h"    // Para usar o memory_copy


// Criando o ponteiro de 1 byte (char*) apontando para o endereço de memória da VGA (0xb8000)
static volatile char* video_memory = (volatile char*) 0xb8000;

// Variáveis Globais que irão guardar a posição do cursor
static int cursor_row = 0;
static int cursor_col = 0;

void update_cursor() {
    // Calcula a posição linear por meio apenas do índice
    unsigned short position = (cursor_row * MAX_COLS) + cursor_col;

    // Diz ao CRTC (Nossa porta 0x3D4) que vamos enviar o byte ALTO (Comando 14)
    port_byte_out(0x03D4, 14);
    // Envia o Byte alto da posição (Porta 0X3D5) empurrand os bits 8 casa para frente (direita)
    port_byte_out(0x3D5, (unsigned char)(position >> 8));

    // Diz ao CRTC (porta 0x3D4) que vamos enviar o byte BAIXO (comando 15)
    port_byte_out(0x3D4, 15);
    // Envia o byte BAIXO da posição (mascarando com 0xFF para pegar só o final)
    port_byte_out(0x3D5, (unsigned char)(position & 0xFF));
}

// Função que arrasta todas as linhas da tela um andar para cima
void scroll() {
    // Calcualr o tamanho exato de uma linha em bytes:
    int row_bytes = MAX_COLS * 2;

    // Copiamos tudo da Linha 1 em diante, e colamos por cima da Linha 0
    // Lógica: Copie de (video_memory + 160) e cole em (video_memory)
    memory_copy(
        (char*)(video_memory + row_bytes),        // Origem (A partir da Linha 1)
        (char*)video_memory,                      // Destino (Linha 0)
        (MAX_ROWS - 1) * row_bytes                // Quantidade (24 linhas inteiras)
    );

    // 3. Apagamos a última linha para receber o novo texto
    int last_line_offset = (MAX_ROWS - 1) * row_bytes;
    for (int i = 0; i < row_bytes; i += 2) {
        video_memory[last_line_offset + i] = ' ';
        video_memory[last_line_offset + i + 1] = WHITE_ON_BLACK;
    }

    // 4. Resetamos o cursor
    cursor_row = MAX_ROWS - 1;
    cursor_col = 0;
    update_cursor();
}

// Função para limpar a tela inteira
void clear_screen() {
    // 80 COlunas * 25 linhas * 2 bytes por caractere = 4000
    for (int i = 0; i < MAX_COLS * MAX_ROWS *2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i+1] = WHITE_ON_BLACK;
    }

    // Zera o cursor para o topo novamente
    cursor_row = 0;
    cursor_col = 0;
    update_cursor();
}

void print_char(char character, char color) {
    if(character == '\n') {
        cursor_row++;
        cursor_col = 0;
        if(cursor_row >= MAX_ROWS) {scroll();}
        update_cursor();
        return; //O \n não precisa ser imprimido na tela, por isso saímos da função
    }

    // Calcula a posição na "fita" de memória
    int offset = (cursor_row * MAX_COLS + cursor_col) * 2;

    // O primeiro byte vai para o caracter
    video_memory[offset] = character;
    // O segundo byte, vai para a cor (o byte seguinte)
    video_memory[offset+1] = color;

    //Move o cursor uma casa para o lado
    cursor_col++;

    // Se a coluna passou do limite (80), quebra a linha automaticamente
    if (cursor_col >= MAX_COLS) {
        cursor_col = 0;
        cursor_row++;
        update_cursor();
    }

    // NOVA TRAVA DE SEGURANÇA: Se a linha passar de 24, rola a tela!
    if (cursor_row >= MAX_ROWS) {
        scroll();
    }
    update_cursor();
}

// Mini "gambiarra" de um print: passa letra por letra no print_char
void print(char* message, char color) {
    int i = 0;
    while (message[i] != '\0') {
        print_char(message[i], color);
        i++;
    }
}