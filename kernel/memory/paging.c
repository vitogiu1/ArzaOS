#include "paging.h"
#include "pmm.h"
#include "../../drivers/screen.h"
#include "../../libc/string.h"

// O diretório mestre (PD)
page_directory_t *kernel_directory;

static void load_page_directory(uint32_t *directory_addr) {
  // Colocar o endereço do Diretório no registrador CR3
  __asm__ __volatile__("mov %0, %%cr3":: "r"(directory_addr));

  // Ler o CR0, ligar o bit 31 (Paging Enable)
  uint32_t cr0;
  __asm__ __volatile__("mov %%cr0, %0": "=r"(cr0));
  cr0 |= 0x80000000;
  __asm__ __volatile__("mov %0, %%cr0":: "r"(cr0));
}

// MAPEAMENTO
void vmm_map_page(uint32_t virtual_addr, uint32_t physical_addr) {
  // Saber em qual entrada de tabelas mexer, "fatiamos" o endereço virtual
  virtual_addr /= PAGE_SIZE;

  // Os bits 22 a 31 dizem qual o índice no Diretório (0 a 1023)
  uint32_t pd_index = virtual_addr / 1024;

  // Os bits de 12 a 21 dizem qual o índice na TAblea de Paginas (0 a 1023)
  uint32_t pt_index = virtual_addr % 1024;

  if(!(kernel_directory->entries[pd_index] & PTE_PRESENT)) {
    // A tabela ainda não existe, então alocamos ela na RAM
    uint32_t physic_table = pmm_alloc_frame();

    // Zera a nova tablea
    page_table_t *new_table = (page_table_t *)physic_table;
    for (int i = 0; i < 1024; i++) new_table->entries[i] = 0;

    // Registrar a nova tablea no diretório (Endereço + flags)
    kernel_directory->entries[pd_index] = (physic_table) | PTE_PRESENT | PTE_READ_WRITE | PTE_USER;
  }

  // A entrada no Diretório aponta para o endereço da Tabela
  uint32_t address_table = kernel_directory->entries[pd_index] & 0xFFFFF000;
  page_table_t *table = (page_table_t *)address_table;

  // Registrar o mapeamento na Tabela
  table->entries[pt_index] = (physical_addr) | PTE_PRESENT | PTE_READ_WRITE | PTE_USER;
}

// O inicio da paginação
void vmm_init() {
  // Inicialização de um bloco de RAM para ser o diretório mestre PD
  uint32_t physic_pd = pmm_alloc_frame();
  kernel_directory = (page_directory_t *)physic_pd;

  // Zera o diretório por segurança
  for(int i = 0; i < 1024; i++) kernel_directory->entries[i] = 0;

    // O Mapeamento de Identidade (A Sobrevivência do Kernel)
    // Se a Paginação ligar agora, a máquina trava. O kernel está rodando
    // no endereço 0x1000. É necessário dizer à CPU que o endereço virtual 0x1000
    // ainda aponta para o físico 0x1000.
    // É necessário mapear os primeiros 4 Megabytes de memória 1-para-1.
    // O Vídeo (0xB8000), o Kernel (0x1000) e as Pilhas ficarão a salvo.
    for (uint32_t i = 0; i < (1024 * PAGE_SIZE); i += PAGE_SIZE) {
      vmm_map_page(i,i);
    }

    load_page_directory((uint32_t *)kernel_directory);
    
    print("Gerenciador de Memoria Virtual (VMM) Ativado\n", GREEN_ON_BLACK);
}
