#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>

// A lista ligada do HEAP
// Cada pedaço de memória no Heap terá esses dados antes dos dados reais
typedef struct heap_segment {
    uint32_t size;                  // Tamanho dos dados do utilizador
    uint8_t is_free;                // 1 = Livre, 0 = Ocupado
    struct heap_segment *next;      // Ponteiro para o próximo bloco de memória na rua
    struct heap_segment *prev;      // Ponteiro para o bloco anterior
} heap_segment_t;

void init_kernel_heap();
void *kmalloc(uint32_t size);
void kfree(void *ptr);

#endif