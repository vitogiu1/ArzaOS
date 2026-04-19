#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

// Forçando a struct a ter exatamente 24 bytes, sem espaços vazios
typedef struct __attribute__((packed)) {
    uint64_t base_address;       // 8 bytes: Onde o bloco de RAM começa
    uint64_t length;            // 8 bytes: Qual o tamanho do bloco
    uint32_t type;              // 4 bytes: retorna 1 = RAM livre, 2 = Reservado
    uint32_t acpi_attrs;        // 4 bytes: atributos extras
} e820_entry_t;

void read_memory_map();

#endif