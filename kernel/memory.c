#include "memory.h"
#include "../drivers/screen.h"
#include "../libc/string.h"

void read_memory_map() {
    // No bootloader, foi carregado o total de linhas que ele tem que ler nos primeiros 4 bytes a partir
    // do endereço de memória 0x8004
    uint32_t *line_counter = (uint32_t *)0x8000;
    uint32_t total_lines = *line_counter;

    // A lista com os dados em si, começa depois dos 4 bytes, em 0x8004
    e820_entry_t *map = (e820_entry_t *)0x8004;

    print("\n--- MAPA DE MEMORIA FISICA (E820) --\n", BLUE_ON_BLACK);

    print("Regioes detectadas pela BIOS: ", WHITE_ON_BLACK);
    char s_total[10];
    itoa(total_lines, s_total);
    print(s_total, WHITE_ON_BLACK);
    print("\n", WHITE_ON_BLACK);

    // Vasculhar a lista e procurar pela RAM Livre
    for (uint32_t i = 0; i < total_lines; i++) {
        if(map[i].type == 1) {
            print("Regiao Livre encontrada! Tamanho (Bytes):", GREEN_ON_BLACK);

            // Será imprimido no momento apenas a metade inferior do length, já que a função ITOA do kernel
            // é 32 bits, e o length é de 64
            uint32_t low_height = (uint32_t)(map[i].length & 0xFFFFFFFF);

            char s_ht[16];
            itoa(low_height, s_ht);
            print(s_ht, WHITE_ON_BLACK);
            print("\n", WHITE_ON_BLACK);
        } else {
            print("Regiao reservada ignorada.\n", GRAY_ON_BLACK);
        }
    }
    print("-------------------------------------\n", BLUE_ON_BLACK);
}