#include "pic.h"
#include "ports.h"

// Função essencial: O "End of Interrupt" (EOI)
// Se não mandarmos isso no final de cada interrupção, o PIC trava e não manda a próxima tecla!
void pic_send_eoi(unsigned char irq) {
    if(irq >= 8) {
        port_byte_out(PIC2_COMMAND, 0x20); // Avisa o Slave
    }
    port_byte_out(PIC1_COMMAND, 0x20);     // Avisa o Master
}

void pic_remap(int offset1, int offset2) {
    unsigned char a1, a2;
    
    // Salva as máscaras atuais (quais portas estavam ligadas/desligadas)
    a1 = port_byte_in(PIC1_DATA);
    a2 = port_byte_in(PIC2_DATA);

    // (ICW1): Acorda os dois PICs e diz "Preparem-se para reconfiguração"
    port_byte_out(PIC1_COMMAND, ICW1_INIT);
    port_byte_out(PIC2_COMMAND, ICW1_INIT);

    // (ICW2): O Remapeamento! (Onde as IRQs vão morar na IDT)
    port_byte_out(PIC1_DATA, offset1); // Master vai para 32 (0x20)
    port_byte_out(PIC2_DATA, offset2); // Slave vai para 40 (0x28)

    // (ICW3): Mostra onde eles estão conectados fisicamente (Cascata)
    port_byte_out(PIC1_DATA, 0x04); // Diz ao Master que o Slave está na IRQ 2
    port_byte_out(PIC2_DATA, 0x02); // Diz ao Slave sua própria identidade (2)

    // (ICW4): Configura o modo de operação para processadores 8086/x86
    port_byte_out(PIC1_DATA, 0x01);
    port_byte_out(PIC2_DATA, 0x01);

    // Restaura as máscaras antigas de ligar/desligar
    port_byte_out(PIC1_DATA, a1);
    port_byte_out(PIC2_DATA, a2);
}