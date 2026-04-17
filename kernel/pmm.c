#include "pmm.h"
#include "memory.h"
#include "../drivers/screen.h"
#include "../libc/string.h"

// quantidade responsiva de "gavetas" de 32 bits, contendo 4kb de informação da RAM em cada bit "bitmap"
// Rastreando a ram inteira da máquina
static volatile uint32_t *memory_bitmap = (uint32_t *)0x4B000;

// Quantidade de frames que a máquina tem
static uint32_t max_frames = 0;

// ------- FUNÇÕES PRIVADAS DE MANIPULAÇÃO DE BITS -------

// Liga um bit (Transforma o 0 em 1) -> Para marcar como ocupado
static void bitmap_set(uint32_t frame) {
    uint32_t bitmap = frame / 32;        // Em qual dos inteiros ele está
    uint32_t exact_bit = frame % 32;     // Em qual posição (de 0 a 31) dentro do inteiro está

    memory_bitmap[bitmap] |= (1U << exact_bit);
}

// Desliga um bit (Transforma o 1 em 0) -> Para Marcar como livre
static void bitmap_clear(uint32_t frame) {
    uint32_t bitmap = frame / 32;
    uint32_t exact_bit = frame % 32;

    memory_bitmap[bitmap] &= ~(1U << exact_bit);
}

// Testa se um bit é 1 (Retorna verdadeiro se estiver ocupado)
static int bitmap_test(uint32_t frame) {
    uint32_t bitmap = frame / 32;
    uint32_t exact_bit = frame % 32;

    // Isola o bit e verifica se é diferente de zero
    return (memory_bitmap[bitmap] & (1U << exact_bit)) != 0;
}

// ------- INICIALIZAÇÃO DO GERENCIADOR DE MEMÓRIA -------

void pmm_init() {
    // Pegando os dados exatos que o bootloader deixou a partir de 0x40000
    uint32_t total_lines = *((uint32_t *)0x8000);
    e820_entry_t *map = (e820_entry_t *)0x8004; 

    uint32_t max_address = 0;

    // Descobrir onde a RAM física termina
    for (uint32_t i = 0; i < total_lines; i++) {
        if(map[i].type == 1) {
            uint32_t end = (uint32_t)map[i].base_address + (uint32_t)map[i].length;
            if(end > max_address) {
                max_address = end;
            }
        }
    }

    // Calcular o total de RAM da máquina real
    max_frames = max_address / PMM_FRAME_SIZE;

    // Quantidade de inteiros de 32 bits necessários para rastrear a RAM
    uint32_t bitmap_length = max_frames / 32;

    // Abordagem Pessimista: Trancar a memória té o limite real da máquina
    for (uint32_t i = 0; i < bitmap_length; i ++) {
        memory_bitmap[i] = 0xFFFFFFFF;
    }

    // Analise do mapa e820 e liberar apenas a ram que foi garantida pelo mapa vindo da BIOS
    for (uint32_t i = 0; i < total_lines; i++) {
        if(map[i].type == 1) {
            uint32_t start = (uint32_t)map[i].base_address;
            uint32_t address_length = (uint32_t)map[i].length;
            uint32_t end = start + address_length; 

        // Para cada frame (blocos de 4kb) dentro desta região livre, marcamos como LIVRE (0)
        for (uint32_t address = start; address < end; address += PMM_FRAME_SIZE ) {
            uint32_t frame = address / PMM_FRAME_SIZE;
            bitmap_clear(frame);
        }
        }
    }

    // Trancar o primeiro megabyte (256 frames)
    // Pois o Kernel (0x1000) e a tela VGA (0xB8000) estão aqui
    for(uint32_t i = 0; i < 256; i++) {
        bitmap_set(i);
    }

    // Como o bitmap está em 0x4B000, é necessário calcular a quantidade de blocos
    // Que ele ocupa e marcá-los como OCUPADOS, senão ele pode ser sobreescrito
    uint32_t bitmap_frames = ((bitmap_length * 4) / PMM_FRAME_SIZE) + 1;
    uint32_t start_frame = 0x4B000 / PMM_FRAME_SIZE;

    for(uint32_t i = 0; i < bitmap_frames; i ++) {
        bitmap_set(start_frame + i);
    }


    print("\n--- DIAGNOSTICO PMM ---\n", YELLOW_ON_BLACK); 

    print("Linhas Lidas: ", WHITE_ON_BLACK);
    char s_linhas[16];
    itoa(total_lines, s_linhas);
    print(s_linhas, RED_WHITE);
    print("\n", WHITE_ON_BLACK);

    print("Max Frames Calculados: ", WHITE_ON_BLACK);
    char s_frames[16];
    itoa(max_frames, s_frames);
    print(s_frames, RED_WHITE);
    print("\n", WHITE_ON_BLACK);

    print("Gerenciador de Memoria fisica (PMM) ativado,\n", GREEN_ON_BLACK);
}

// Alocar e Liber memória (API publica)
uint32_t pmm_alloc_frame() {
    // Vaculhar as "gavetas" uma por uma
    for (uint32_t i = 0; i < max_frames / 32; i++) {
        // Verifica se a memória não está 100% cheia
        if(memory_bitmap[i] != 0xFFFFFFFF) {

            for(int j = 0; j < 32; j++) {
                uint32_t bit_mask = 1U << j;

                if(!(memory_bitmap[i] & bit_mask)) {
                    uint32_t frame_free = (i * 32) + j;
                    bitmap_set(frame_free); // Marcar como usado para ninguém mais usar

                    return frame_free * PMM_FRAME_SIZE; // Retornar o endereço físico na RAM
                }
            }
        }
    }

    // Se o loop terminou e não retornou, o sistema está sem ram
    print("PANIC: Sistema sem memoria RAM fisica!\n", RED_ON_BLACK);
    while(1); 
    return 0;
}

// Devolve um frame para o sistema
void pmm_free_frame(uint32_t frame_address) {
    uint32_t frame = frame_address / PMM_FRAME_SIZE;
    bitmap_clear(frame); 
}