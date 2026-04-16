[bits 32]
section .text

extern isr_handler ; A função central em C

; MACRO: Cria um guarda-costas que NÃO tem código de erro
%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    push byte 0  ; Empilha um erro "falso" para manter a struct alinhada
    push byte %1 ; Empilha o Número da Interrupção (Porta)
    jmp isr_common_stub
%endmacro

; MACRO: Cria um guarda-costas que já possuí código de erro da CPU
%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    ; A CPU já empilhou o código de erro sozinha, então só basta emiplhar o Número da Porta
    push byte %1
    jmp isr_common_stub
%endmacro

; GERANDO OS 32 GUARDA-COSTAS AUTOMATICAMENTE
; (As portas 8, 10, 11, 12, 13, 14 e 21 geram códigos de erro segundo a Intel)
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
    
    ; Chama o nosso Despachante em C!
    call isr_handler
    
    ; Restaura tudo de volta ao normal
    pop eax 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    
    add esp, 8      ; Limpa o Número do Erro e o Número da Interrupção da pilha
    iret            ; Destranca a CPU!