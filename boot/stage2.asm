[bits 16]
[org 0x1000] ; O fim do estágio 1, trouxe para cá

stage2_start:
    ; LOG de segurança, imprime '2' para garantir que o estágio 2 foi iniciado
    mov ah, 0x0e
    mov al, '2'
    int 0x10

; ------ CARREGAR O MAPA DE MEMÓRIA (E820) ------
load_memory_map:
    mov di, 0x8004           ; Destiny Index, a listá começará em 0x8004, pois de 8000 até 0x8003, estará o nosso contador de linhas
                             ; Essencial para o kernel em C saber onde termina a leitura da memória RAM
    xor ebx, ebx             ; ebx = 0 (A BIOS exige que na primeira chamada ele seja 0)
    xor ebp, ebp             ; bp = 0 (Vamos usar o registrador bp como nosso contador de linhas)
    mov edx, 0x534D4150      ; A senha "SMAP" de hexadecimal para ASCII, para a BIOS ter certeza do que estamos pedindo (System Memory Address Map)

.do_e820:
    mov eax, 0xe820         ; Chamando a função E820: registrar o mapa da memória RAM
    mov ecx, 24             ; tamanho do buffer, que será de 24 bytes, cada informação sobre a RAM
    int 0x15                ; Interrupção da Placa mãe

    jc e820_fail             ; Caso tenha finalizado ou dado erro, o carry flag é 1, e a condicional é verdadeira
    cmp eax, 0x534D4150     ; Se a BIOS confirmou o SMAP, ela registra no eax
    jne e820_fail            ; Se ela não confirmou, é por que deu erro.

    ; Se a BIOS está aqui, é por que está tudo certo e escrever os 24 bytes com sucesso
    add di, 24               ; Movemos o ponteiro da memória para o próximo endereço que será preenchido
    inc ebp                  ; Adicionamos mais um no bp, para indicar no final, quantas linhas terá

    test ebx,ebx            ; verifica se a BIOS zerou o ebx
    jne .do_e820             ; Se não, significa que ainda tem mais linhas, então tem que seguir copiando.


.e820_end:
    ; O Loop terminou. Agora salvamos quantas linhas foram encontrada
    mov dword [0x8000], ebp    ; Guarda a quantidade de linhas encontradas nos primeiros 4 bytes do endereço que está sendo trabalhado.
    
    ; Zerar o ES para não falhar na leitura de disco
    xor ax, ax
    mov es, ax 
    
    mov ah, 0x0e
    mov al, 'M'
    int 0x10

; ------ LIGAR LINHA A20 (FAST A20 GATE) ------
enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al

; ------ ENTRANDO NO MODO PROTEGIDO (32 BITS)
enter_32bits:
    cli                     ; Desligar as interrupções
    lgdt [gdt_descriptor]   ; Carrega a Tabela GDT de 32-bits

    mov eax, cr0
    or eax, 0x1             ; Ativa o Bit 0 do registrador CR0 (Protection Enable)
    mov cr0, eax            

    ; "Far Jump" para limpar o cérebro da CPU (Pipeline)
    ; 0x08 é o nosso Segmento de Código na GDT.
    jmp CODE_SEG:start_protected_mode


e820_fail:
    mov ah, 0x0e
    mov al, 'E'
    int 0x10
    jmp $

; -------- MODO 32 BITS -------
[bits 32]
start_protected_mode:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

    ; Imprimir no VGA a letra 3, pois não existem mais interrupções da BIOS
    mov byte [0xB8000], '3'
    mov byte [0xB8001], 0x0A

    jmp $

; ---------- GDT (GLOBAL DESCRIPTOR TABLE) ----------
gdt_start:
    dq 0x0                  ; Segmento Nulo (A CPU exige que o primeiro seja zero)
gdt_code:                   ; Segmento 0x08 (Código)
    dw 0xFFFF, 0x0          ; Limite e Base
    db 0x0, 10011010b       ; Permissões (Código Executável)
    db 11001111b, 0x0       ; Flags
gdt_data:                   ; Segmento 0x10 (Dados)
    dw 0xFFFF, 0x0          
    db 0x0, 10010010b       ; Permissões (Dados Leitura/Escrita)
    db 11001111b, 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Tamanho exato da GDT
    dd gdt_start                ; Endereço de onde ela começa

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start