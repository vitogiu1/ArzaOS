[bits 32]
section .text

extern isr_handler ; A função central em C

; MACRO: Cria um salvaguardas que NÃO tem código de erro
%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    push byte 0  ; Empilha um erro "falso" para manter a struct alinhada
    push byte %1 ; Empilha o Número da Interrupção (Porta)
    jmp isr_common_stub
%endmacro

; MACRO: Cria um salvaguardas que já possuí código de erro da CPU
%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    ; A CPU já empilhou o código de erro sozinha, então só basta emiplhar o Número da Porta
    push byte %1
    jmp isr_common_stub
%endmacro

; GERANDO AS 32 SALVAGUARDAS AUTOMATICAMENTE
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_ERRCODE   21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31


; O DESPACHANTE (A Esteira de Montagem)
isr_common_stub:
    pusha           ; Salva edi,esi,ebp,esp,ebx,edx,ecx,eax
    
    mov ax, ds      ; Salva o Segmento de Dados antigo
    push eax        
    
    mov ax, 0x10    ; Carrega o Segmento de Dados do Kernel (DATA_SEG)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call isr_handler
    add esp, 4

    ; Restaura tudo de volta ao normal
    pop eax 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    
    add esp, 8      ; Limpa o Número do Erro e o Número da Interrupção da pilha
    iret            ; Destranca a CPU!

; IRQs - Interrupções de Hardware (Teclado e Relógio)
extern irq_handler

; Macro para criar as portas dos periféricos
%macro IRQ 2
  global irq%1
  irq%1:
    push byte 0     ; Periféricos não mandam código de erro, então será empurrado um zero falso
    push byte %2    ; Empurrando a Porta (Ex: 32 para o Relógio, 33 para o Teclado)
    jmp irq_common_stub
%endmacro

; Criando os salva-guardas dos Periféricos de Hardware (IRQs)
IRQ 0, 32   ; Relógio (Timer)
IRQ 1, 33   ; Teclado
IRQ 2, 34   ; Cascata PIC
IRQ 3, 35   ; COM2
IRQ 4, 36   ; COM1
IRQ 5, 37   ; LPT2
IRQ 6, 38   ; Disquete
IRQ 7, 39   ; LPT1
IRQ 8, 40   ; CMOS (Relógio em tempo real)
IRQ 9, 41   ; Livre
IRQ 10, 42  ; Livre
IRQ 11, 43  ; Livre
IRQ 12, 44  ; Mouse (Mouse PS/2)
IRQ 13, 45  ; Coprocessador
IRQ 14, 46  ; Disco Rígido Primário (IDE)
IRQ 15, 47  ; Disco Rígido Secundário (IDE)

; A Esteira de Montagem dos Periféricos
irq_common_stub:
    pusha           
    
    mov ax, ds      
    push eax        
    
    mov ax, 0x10    
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp 
    call irq_handler ; Chama a função em C
    add esp, 4
    
    pop eax 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    
    add esp, 8      
    iret