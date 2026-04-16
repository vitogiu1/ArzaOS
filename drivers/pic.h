#ifndef PIC_H
#define PIC_H

#include <stdint.h>

// Endereços físicos do chip na placa-mãe
#define PIC1_COMMAND 0x20 // Master
#define PIC1_DATA    0x21 
#define PIC2_COMMAND 0xA0 // Slave
#define PIC2_DATA    0xA1 

// Comando de Inicialização (ICW1)
#define ICW1_INIT    0x11 // Modo de inicialização obrigatório

// Função para iniciar o remapeamento
void pic_remap(int offset1, int offset2);

// Função para avisar o PIC que terminamos de processar a tecla
void pic_send_eoi(unsigned char irq);

#endif