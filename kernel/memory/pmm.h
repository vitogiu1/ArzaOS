#ifndef PMM_H
#define PMM_H

#include <stdint.h>

// Todo bloco de memória tem 4096 bytes para analisar
#define PMM_FRAME_SIZE 4096

// Inicializa o mapa lendo a lista e820
void pmm_init();

// Pede 1 frame (de 4kb) de memória ram livre e retorna o endereço físico dele
uint32_t pmm_alloc_frame();

// Devolve o frame para o sistema (marca como livre)
void pmm_free_frame(uint32_t frame_address);

#endif