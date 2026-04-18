#include "heap.h"
#include "pmm.h"
#include "paging.h"
#include "../drivers/screen.h"

// Onde começa o HEAP na memória virtual
// Ele será inserido num endereço redondo e seguro: 0x1000000
#define KERNEL_HEAP_START 0x1000000

// O primeiro bloco da lista ligada
heap_segment_t *heap_head = 0;

// Rasteria onde o Heap termina na memória virtual
uint32_t current_heap_end = KERNEL_HEAP_START;

// Ligar o HEAP
void init_kernel_heap() {
    // Pedido do bloco físico completo ao PMM
    uint32_t block = pmm_alloc_frame();

    // Ligar a Paginação para o endereço
    vmm_map_page(KERNEL_HEAP_START, block);

    // Verificar a fronteira do heap
    current_heap_end = KERNEL_HEAP_START + PAGE_SIZE;

    heap_head = (heap_segment_t *)KERNEL_HEAP_START;

    // Preenchemos os dados. (O tamanho livre é 4096 bytes menos o espaço que o próprio cabeçalho ocupa)
    heap_head->size = PAGE_SIZE - sizeof(heap_segment_t);
    heap_head->is_free = 1;
    heap_head->next = 0;
    heap_head->prev = 0;

    print("Kernel Heap iniciado com suceso!\n", GREEN_ON_BLACK);
}

// Split do bloco
// Se um bloco livre tem 4000 bytes, e é pedido 50, corta-se em dois:
// Um bloco ocupado de 50 bytes, e um novo livre de 3900 bytes
static void split_segment(heap_segment_t *segment, uint32_t size_requested) {
    // o local que irá nascer o novo cabeçalho será a soma do endereço atual + tamanho do cabeçalho + tamanho dos pedidos
    uint32_t new_address = (uint32_t)segment + sizeof(heap_segment_t) + size_requested;

    heap_segment_t *new_segment_free = (heap_segment_t *)new_address;

    new_segment_free->is_free = 1;
    // O tamanho do novo retalho é o que sobrou, tirando o espaço do novo cabeçalho
    new_segment_free->size = segment->size -size_requested -sizeof(heap_segment_t);

    new_segment_free->next = segment->next;
    new_segment_free->prev = segment;

    if(segment->next != 0) {
        segment->next->prev = new_segment_free;
    }

    segment->next = new_segment_free;
    segment->size = size_requested;
}

void *kmalloc(uint32_t size) {
    if (size == 0) return 0;

    // Iremos arredondar qualquer pedido para o múltiplo de 4 mais próximos
    // para evitar trabalhar com tamanhos ímpares
    // ANALISAR OUTRA SAÍDA FUTURAMENTE
    uint32_t remainder = size % 4;
    if (remainder != 0) {
        size += (4 - remainder);
    }

    heap_segment_t *current = heap_head;

    while (current != 0) {
        // encontrar um bloco livre, e que cabe o que está sendo pedido
        if(current->is_free == 1 && current->size >= size) {

            // Analisar se o bloco é muito maior do que precisa, se sim, dar um spli
            if(current->size > (size + sizeof(heap_segment_t) + 4)) {
                split_segment(current, size);
            }
            
            // Marca como ocupado
            current->is_free = 0;

            // O utilizado não quer saber do cabeçalho, então pula-se um para pular o cabeçalho (+1 pula o tamanho da struct)
            return (void *)(current + 1);
        }

        current = current->next;
    }

    // Expansão do heap

    //Se o while acabou, não há espaço, então deve-se ir ao próximo bloco

    // Encontrar p último endereço do heap atual
    heap_segment_t *last = heap_head;
    while (last->next != 0) {
        last = last->next;
    }

    // Quantas páginas de 4KB é necessário
    uint32_t pages_needed = (size + sizeof(heap_segment_t)) / PAGE_SIZE + 1;

    // Busca as paginas pedindo ao PMM para mapear no VMM
    for (uint32_t i = 0; i < pages_needed; i++) {
        uint32_t new_block = pmm_alloc_frame();
        vmm_map_page(current_heap_end, new_block);
        current_heap_end += PAGE_SIZE; // Empurra o fim do bloco pra frente
    }

    // Criar um novo head livre
    // Logo após ao fim do último segmento
    uint32_t new_address = (uint32_t) last + sizeof(heap_segment_t) + last->size;
    heap_segment_t *new_segment = (heap_segment_t *)new_address;

    new_segment->is_free = 1;
    // O tamanho é o total de páginas novas menos o tamanho do head
    new_segment->size = (pages_needed * PAGE_SIZE) - sizeof(heap_segment_t);
    new_segment->next = 0;
    new_segment->prev = last;

    // Conectamos o novo com o antigo
    last->next = new_segment;

    // Limpeza: Se o último vizinho antigo já estava livre, os dois serão fundidos
    if(last->is_free) {
        last->size += sizeof(heap_segment_t) + new_segment->size;
        last->next = 0;
    }

    // Agora, tentamos alocar novamente
    return kmalloc(size);
}

void kfree(void *ptr) {
    if(ptr == 0) return;

    // Encontrar o cabeçalho
    // O utilizador fornece o ponteiro dos dados
    // Então, deve ser feito um "cast" para o tipo do cabeçalho, e subtrair 1 
    // Pois o cabeçalho está logo antes do endereço do dado que armazenamos
    heap_segment_t *segment = (heap_segment_t *)ptr - 1;

    // Marcar como livre
    segment->is_free = 1;

    // Fusão para a frente
    // Se o retalho vizinho da frente também estiver livre, destrui-se a "parede"
    // entre ambos, e ocorrerá a união de tudo junto
    if(segment->next != 0 && segment->next->is_free == 1) {
        // Somamos o nosso tamanho + tamanho do cabeçalho + tamanho do próximo
        segment->size = segment->size +sizeof(heap_segment_t) + segment->next->size;

        // o Próximo segmento a ser vizinho do vizinho
        segment->next = segment->next->next;

        // Se houver algo lá na frente, avisa-se onde está o segmento atual
        if(segment->next != 0) {
            segment->next->prev = segment;
        }

        // Fusão com o anterior
        if(segment->prev != 0 && segment->prev->is_free == 1) {
            heap_segment_t * prev_segment = segment->prev;

            prev_segment->size = prev_segment->size + sizeof(heap_segment_t) + segment->size;
            prev_segment->next = segment->next;

            if(segment->next != 0) {
                segment->next->prev = prev_segment;
            }
        }
    }
}