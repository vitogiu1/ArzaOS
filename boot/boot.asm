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

; ------ DETECTAR A MEMÓRIA RAM (E820) ------
    mov si, MSG_E820
    call print_string

    mov di, 0x5004          ; Destiny Index, a listá começará em 0x5004, pois de 5000 até 5003, estará o nosso contador de linhas
                            ; Essencial para o kernel em C saber onde termina a leitura da memória RAM
    xor ebx, ebx            ; ebx = 0 (A BIOS exige que na primeira chamada ele seja 0)
    xor ebp, ebp              ; bp = 0 (Vamos usar o registrador bp como nosso contador de linhas)

do_e820:
    mov eax, 0xe820         ; Chamando a função E820: registrar o mapa da memória RAM
    mov ecx, 24             ; tamanho do buffer, que será de 24 bytes, cada informação sobre a RAM
    mov edx, 0x534D4150     ; A senha "SMAP" de hexadecimal para ASCII, para a BIOS ter certeza do que estamos pedindo (System Memory Address Map)
    int 0x15                ; Interrupção da Placa mãe

    jc e820_end             ; Caso tenha finalizado ou dado erro, o carry flag é 1, e a condicional é verdadeira
    cmp eax, 0x534D4150     ; Se a BIOS confirmou o SMAP, ela registra no eax
    jne e820_end            ; Se ela não confirmou, é por que deu erro.

    ; Se a BIOS está aqui, é por que está tudo certo e escrever os 24 bytes com sucesso
    add di, 24              ; Movemos o ponteiro da memória para o próximo endereço que será preenchido
    inc ebp                  ; Adicionamos mais um no bp, para indicar no final, quantas linhas terá

    test ebx,ebx            ; verifica se a BIOS zerou o ebx
    jne do_e820             ; Se não, significa que ainda tem mais linhas, então tem que seguir copiando.

e820_end:
    ; O Loop terminou. Agora salvamos quantas linhas foram encontrada
    mov dword [0x5000], ebp    ; Guarda a quantidade de linhas encontradas nos primeiros 4 bytes do endereço que está sendo trabalhado.

; ------ LER O KERNEL DO DISCO PARA A RAM ------
    ; Configurar onde a BIOS vai despejar os dados na memória RAM
    mov si, MSG_DISK
    call print_string

    mov bx, 0x1000           ; bx = Endereço de Destino (onde o Kernel estará carregado)

    ; Configurar as coordendas de Disco (CHS)
    mov ah, 0x02             ; Função 0x02 para ler os setores do disco
    mov al, 32               ; Quantos setores irá ler: 32 (16 kb)
    mov ch, 0                ; Cilindro 0
    mov dh, 0                ; Cabeça (Head) 0
    mov cl, 2                ; Setor Inicial = 2 (o índice de contagem dos setores em x86 começa em 1)
    mov dl, [BOOT_DRIVE]     ; O disco que será lido, que foi carregado no ínicio
    int 0x13                 ; Acionando a interrupção da BIOS de leitura de disco

    ; Verificação de Segurança
    jc disk_error            ; Se a CArry Flag Ligar, deu falha de hardware, e o programa deve parar de executar
    cmp al, 32               ; Confirma se a BIOS leu os 32 setores completo
    jne disk_error           ; Se leu menos, o Kernel veio corrompido ou algo assim

    ; ------ ATIVAR A LINHA A20 ------
    ; Pede à BIOS para ligar a porta A20 (permitindo acessar além de 1MB de RAM)
    mov si, MSG_PROT
    call print_string

    mov ax, 0x2401
    int 0x15

    jmp transition 

disk_error:
    mov si, MSG_ERROR_DISK
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



    ; ------ TRANSIÇÃO PARA 32 BITS ------
transition: 
    cli                      ; Clear Interrupt Flag Desliga as interrupções da BIOS

    lgdt [gdt_descriptor]    ; Carrega a tabela GDT

    ; Adiciona o bit 1 ao primeiro bit do registrador CR0, para indicar que entrará no modo 32 bits
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    ; Far Jump: Limpa a fila de execução de 16 bits
    ; Foçando a CPU a entrar no modo de 32 bits usando o nosso descriptor (CODE_SEG)
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    ; Iniciado no modo protegido (32 bits)
    ; É necessário atualizar todos os registradores de segmentos para o DATA_SEG
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax


    ; Configura a pilha de 32 bits
    mov ebp, 0x9000
    mov esp, ebp

    ; Chamar o Kernel:
    call 0x1000         ; Pula para onde o kernel foi carregado
    jmp $
; ------ VARIÁVEIS GLOBAIS E STRINGS ------
BOOT_DRIVE: db 0
MSG_BOOT:       db "-> Bootloader Iniciado...", 13, 10, 0
MSG_E820:       db "-> Mapeando Memoria RAM (E820)...", 13, 10, 0
MSG_DISK:       db "-> Carregando Kernel do Disco...", 13, 10, 0
MSG_PROT:       db "-> Ativando Linha A20 e Modo 32-bits...", 13, 10, 0
MSG_ERROR_DISK: db "-> Erro identificado ao gravar do disco para a RAM....", 13, 10, 0

; ------ TABELA GDT (Global Descriptor Table) ------

gdt_start: 
    ; Sempre o primeiro descritor do GDT nulo
    dd 0x0
    dd 0x0

; Descritor de código (Offset a partir do 0x08)
gdt_code:
    dw 0xffff           ; Limite (bits 0-15)
    dw 0x0              ; Base (bits 0-15)
    db 0x0              ; Base (bits 16-23)
    db 10011010b        ; Acesso: Presente(1), Ring 0(00), Tipo Código(1), Exec/Read(10)
    db 11001111b        ; Granularidade: 4KB(1), 32-bit(1), Limite(bits 16-19)
    db 0x0              ; Base (bits 24-31)

gdt_data:
    dw 0xffff       ; Limite (bits 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10010010b    ; Acesso: Presente(1), Ring 0(00), Tipo Dados(1), Read/Write(10)
    db 11001111b    ; Granularidade: 4KB(1), 32-bit(1), Limite(bits 16-19)
    db 0x0          ; Base (bits 24-31)

gdt_end:
    ; Geração do GDT Descriptor, o que será enviado para a CPU
gdt_descriptor:
    dw gdt_end - gdt_start - 1      ; Tamanho da GDT (16 bits), pegando do inicio até o final
    dd gdt_start                    ; Engedeço inicial da GDT (32 bits)

; Constantes para facilitar o far jump para 32 bits
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; ------ FINALIZAÇÃO DO BOOTLOADER ------
times 510 - ($ - $$) db 0
dw 0xaa55