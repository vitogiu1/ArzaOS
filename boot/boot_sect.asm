[bits 16]       ; Declarando a inicialização do modo real ao NASM (16 bits)
[org 0x7c00]    ; Avisa para o NASM que a BIOS deve rodar esse código no endereço 0X7C00 na RAM

; ------ INICIALIZAÇÃO DO PROCESSADOR ------

boot_start:
    ; Zerando os segmentos de dados
    xor ax, ax      ; Zerando o ax, comparando bit a bit dele, e retornando zero
    mov ds, ax      ; Data Segment = 0
    mov es, ax      ; Extra Segment = 0
    mov ss, ax      ; Stack Segment = 0


    ; Configura o topo da pilha em um lugar seguro (Longe do kernel)
    mov bp, 0x9000
    mov sp, bp      ; O ponteiro da pilha começa no topo, junto com ela

    ; Salva o número do disco que a BIOS usou para ligar o bootloader
    ; A BIOS deixa esse número no registrador 'dl'
    mov [BOOT_DRIVE], dl

    mov si, MSG_BOOT
    call print_string

; ------ LER O KERNEL DO DISCO PARA A RAM ------
    ; Configurar onde a BIOS vai despejar os dados na memória RAM
    mov si, MSG_DISK
    call print_string

    mov bx, 0x1000           ; bx = Endereço de Destino (onde o estágio 2 estará carregado)

    ; Configurar as coordendas de Disco (CHS)
    mov ah, 0x02             ; Função 0x02 para ler os setores do disco
    mov al, 3                ; Quantos setores irá ler: 3
    mov ch, 0                ; Cilindro 0
    mov dh, 0                ; Cabeça (Head) 0
    mov cl, 2                ; Setor Inicial = 2 (o índice de contagem dos setores em x86 começa em 1)
    mov dl, [BOOT_DRIVE]     ; O disco que será lido, que foi carregado no ínicio
    int 0x13                 ; Acionando a interrupção da BIOS de leitura de disco

    ; Verificação de Segurança
    jc disk_error            ; Se a CArry Flag Ligar, deu falha de hardware, e o programa deve parar de executar
    cmp al, 3                ; Confirma se a BIOS leu os 3 setores completos
    jne disk_error           ; Se leu menos, o Kernel veio corrompido ou algo assim

    mov si, MSG_LOADED
    call print_string

; ------ O GRANDE SALTO ------
    ; O disco foi lido. O Estágio 2 foi carregado exatamente para 0x1000.
    jmp 0x1000

disk_error:
    mov si, MSG_ERROR
    call print_string
    jmp $

print_string:
    pusha               ; Salva todos os registradores para não estragar nada
    mov ah, 0x0e        ; Comando da BIOS: Modo Teletype (Escrever caractere)
.loop:
    mov al, [si]        ; Pega a letra atual para onde o 'si' aponta
    cmp al, 0           ; É o byte zero (Fim da string)?
    je .done            ; Se sim, pula para o final
    int 0x10            ; Imprime a letra na tela!
    inc si              ; Avança para a próxima letra
    jmp .loop           ; Repete o ciclo
.done:
    popa                ; Restaura os registradores
    ret                 ; Volta para onde a função foi chamada


; ------ VARIÁVEIS GLOBAIS E STRINGS ------
BOOT_DRIVE: db 0
MSG_BOOT:       db "-> Bootloader Stage 1...", 13, 10, 0
MSG_LOADED:     db "-> Stage 2 carregado...", 13, 10, 0
MSG_DISK:       db "-> Carregando setor 2 do Disco...", 13, 10, 0
MSG_ERROR:      db "-> Erro identificado ao gravar do disco para a RAM....", 13, 10, 0


; ------ FINALIZAÇÃO DO BOOTLOADER ------
times 510 - ($ - $$) db 0
dw 0xaa55