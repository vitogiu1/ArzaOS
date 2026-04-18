#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

// Tamanho da página: 4kb
#define PAGE_SIZE 4096

// Flags da Intel
//Quando configurar uma página, os últimos 12 bits do endereço
// serão usados para ligar/desligar permissões no hardware

#define PTE_PRESENT 1       // BIT 0: Pergunta se a paginação existe (1 = Sim)
#define PTE_READ_WRITE 2    // BIT 1: Permissão
#define PTE_USER 4          // BIT 2: Ring 

// A tablea de Páginas (Aponta para a RAM física real)
// Contém 1024 entradas. 1024 * 4 = 4kb, o que dá 1 bloco
typedef struct {
    uint32_t entries[1024]; 
} page_table_t;

// O diretório de páginas (Aponta para as Tabelas)
typedef struct {
    uint32_t entries[1024];
} page_directory_t;

// Inicializa a Paginação
void vmm_init();

// Pega um endereço virtual e conecta a um físico
void vmm_map_page(uint32_t virtual_addr, uint32_t physical_addr);

#endif 
