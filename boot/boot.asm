[bits 16]    ; Compatibiliade com o modo real da BIOS de 16 bits
[org 0x7c00] ; setando o endereço de memoria que o código irá iniciar 
; Iniciaremos utilizando o endereço 0x7C00 pois é o primeiro endereço da memória ram reservado ao bootloader

_start:
    ; SETUP DE SEGURANÇA
    ; Primeiro passo é zerar todos os segmentos, para não ter risco de incompatibilidade em algumas BIOS
    xor ax, ax ; AX = 0 (o xor compara bit a bit do, como estamos comparando o mesmo registrador, todos vão dar 00000000, pois é tudo igual)
    mov ds, ax ; Atribui o valor 0 ao Data Segment, vindo de ax
    mov es, ax ; Atribui 0 ao Extra Segment, vindo de ax

    ; instrução CLD (clear direction Flag), garante que o LODSB ante para frente somando na string que utilizaremos
    ; E não para trás.
    cld

    ; ---- LEITURA DE DISCO (int 0x13) interrução de disco
    mov ah, 0x02            ; Função 0x02: Ler setores do disco
    mov al, 50              ; Quantos setores ele terá que ler, 50 para ler tudo de uma vez

    ; Endereçamento CHS (Cylinder 0, Head 0, Sector 2)
    mov ch, 0               ; Cilindro 0
    mov dh, 0               ; Cabeça 0
    mov cl, 2               ; Setor 2 (pois o 1, é o bootloader)

    mov bx, 0x9000          ; Endereço para onde vai do disco para a memória (endereço 0x9000 da memória ram)

    int 0x13                ; Call da interupção de disco, para ler o disco

    ; ---- TRATAMENTO DE ERROS -----
    ; Se der erro, a BIOS avisa ligando a "Carry Flag", pois o if é o jum if carry, um bit de aviso da CPU
    ; Naturalmente na BIOS, existe um código que controla o motor do HD
    ; O código tenta ler o disco, se o disco gerar, a agulhar ler os dados, e os dados forem passados para a memória ram
    ; A BIOS executa o CLC (Clear Carry Flag) antes de devolver o controle para o programa
    ; Forçando o pino do carry a ficar em zero, fazendo com que esse jum if carry não acione

    ; Agora, se o disco estiver arranhado, o setor não existir ou algo assim, a Bios executa a instrução
    ; STC (Set Carry Flag), forçando o pino do carry ficar em 1, acionando o if
    jc .error

    cli ; Desligando as interrupções (BIOS entra em "hibernação")
    ; Chamamos o CLI para que no modo protegido de 32 bits, a BIOS não continue com o seu relógio
    ; que chamava a CPU 18 vezes por segundo, no modo real de 16 bits
    ; O cli corta os fios de comunicação do mundo exterior. O computador fica temporariamente "surdo" para evitar acidentes.


    ; Carregando o Global Descriptor Table (GDT)
    lgdt [gdt_descriptor] ; Carrega (load) o GDT, que é a tabela cheia de bits do final do arquivo
    ; o GDT contém o mapa para a CPU de arquitetura x86. usada para definir os segmentos de memória e os privilégios dela
    ; Estabelecendo as principais regras de acesso à memória (leitura, escrita, execução) 
    ; E os contratos do modo protegido e proteção de acesso à memória

    ; Acionar o registrador CR0
    ; No fundo dos processadores da arquitetura x86, existem registradores chamados de Control Registers
    ; (CR0, CR1, ...), Eles controlam a "anatomia" básica do chip
    ; O bit número zero do CR0, ou seja, o ínicio de tudo, é chamado de PE (Protection Enable)
    ; Isso serve para indicar ao processador, se o modo de proteção está ativado

    ; Então, nós puxamos o valor de CR0 para o registrador EAX
    mov eax, cr0
    or eax, 0x1         ; Forçamos o primeiro bit do EAX ser 1, apenas o primeiro bit, por isso usamos o OR 
    mov cr0, eax        ; Voltamos o novo valor de EAX para o CR0, mudando apenas o valor do primeiro bit para 1
    ; Fazendo com que o falemos para o processador usar o modo de proteção, de 32 bits, deixando para trás o modo real de 16 bits
    ; Vale ressaltar que, aqui só foi possível utilizra o EAX, poiso NASM injeta o byte 0x66, que ativa
    ; o Operand-Size Override, para permitir que neste breve momento, seja possível usar o registrador de 32 bits
    ; FAzemos isso, pois os registradores de controle do processador, só funciona na arquitetura de 32 bits.

    ; O Salto Longo (Far Jump)
    ; Isso limpa a fila de leitura (pipeline) do processador e avisa
    ; para a partir de agora, a CPU usar o segmento de código de 32 bits e ir para o rótulo init_pm"
    jmp CODE_SEG:init_pm ; Estamos dizendo para CPU usar a permissão de mexer na memória com o "crachá" CODE do GDT
    ; Então ele vai para o init_PM com essa permissão. Sem isso, a CPU não permitiria que ele executasse a init_pm, sem "se identificar"
    ; Pois agora estamos no modo protegido de 32 bits.

.error:
    mov si, msg_erro
    call print_string
    jmp $

print_string:
    mov ah, 0x0e

.loop:
    lodsb           ; Puxa o [SI] para o AL e faz SI++
    cmp al, 0       ; Verifica se o caractere que foi para AL é o zero, se for, significa que é o final da frase
    je .done        ; Se for zero, pula para o fim do loop (break)

    int 0x10        ; Se não for zero, continua rodando normalmente, e agora, lança na tela 
                    ; o caractere, por meio do código de interrupção de vídeo da BIOS (0x10)
    jmp .loop       ; Volta para o inicio do loop, para pegar a próxima letra

.done:
    ret

    ; Inicialização do modo protegido (32 bits)
    ; Avisa ao NASM para gerar o código de máquina x86 para 32 bits
    [bits 32]

init_pm:
    ; Aqui, já estamos no modo 32 bits.
    ; A CPU acabou de ser ativada, então precisamos zerar todos os registradores antigos (DS, SS e afins)
    ; e apontá-á-los todos para o nosso novo Segmento de DADOS (DATA_SEG) da GDT.
    ; E aqui, salvamos todos os registradores com a permisão de DATA do GDT
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Mover a Pilha (Stack) de forma segura para o topo da memória livre
    ; para ter espaço entre a pilha e o código do kernel, e nada sobreescrever
    mov ebp, 0x90000 ; Extended Base Pointer, será o nosso ponteiro do topo. Toda vez que o kernel daqui para frente, fizer um push, um call ou criar uma variável, o ESP se move. Ele é o marcador de onde está o último dado guardado.
    mov esp, ebp ; Extended Stack Pointer É o "Ponteiro de Base". Ele é o âncora. Ele serve para o C conseguir achar suas variáveis locais. Ele marca onde a função atual começou, para que o ESP possa se mexer à vontade.
 
    ; O nosso Kernel já foi lido do HD e está na RAM no endereço 0x9000.
    ; Mandamos a CPU para lá!
    jmp 0x9000      ; Executa o Kernel do Setor 2!


; Comandos físicos para a cabeça de impressão virtual, de voltar ao inicio da linha, e pular uma linha
; 13 é o Carriage Return (Volta para o ínicio da linha)
; 10 é o Line Feed, desce uma linha
; 0 é o terminador obrigatório, para indicar ao LODSB que a frase terminou
; --- DADOS ---
msg_ok   db "Kernel lido com sucesso!", 13, 10, 0
msg_erro db "Falha ao ler o disco!", 13, 10, 0

; ------ GDT (Global Descriptor Table) -------
gdt_start:

gdt_null:       ; Primeira linha do GDT é obrigatoriamente zero
    dd 0x0
    dd 0x0

; Dando aesso de leitura e execução para os códigos, mas jamais sobreescrever

gdt_code:           ; Índice 0x08 - Segmento de Código (Base=0, Limite=4GB)
    dw 0xffff       ; Limit (bits 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10011010b    ; Permissões (1=Presente, Privilégio=00, Tipo=Código Exec/Leitura)
    db 11001111b    ; Flags + Limite final
    db 0x0          ; Base (bits 24-31)

; Instrução de dados, podendo ser lidos e escritos, mas jamais executados
gdt_data:           ; Índice 0x10 - Segmento de Dados (Base=0, Limite=4GB)
    dw 0xffff       ; Limit (bits 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10010010b    ; Permissões (Tipo=Dados Leitura/Escrita)
    db 11001111b    ; Flags + Limite final
    db 0x0          ; Base (bits 24-31)

gdt_end:

; Um pequeno ponteiro que diz à CPU qual o tamanho da GDT e onde ela está
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Definimos as constantes com os índices (Offset dentro da tabela)
; São constantes que mostram onde cada segmento começa
CODE_SEG equ gdt_code - gdt_start ; CODE começa no byte 8, em hexadecimal geralmente é 0x08 
DATA_SEG equ gdt_data - gdt_start ; DATA começa no byte 16, em hexadecimal geralmente é 0x10

; Preenchimento
; Para a BIOS Legacy ler, o bootloader precisa ter exatamente 512 bytes
; para preencher o que está faltando para o bootloader, fazemos o times para preencher de bytes '0'
; a partir da instrução 'db 0', até o número 510
; a fórmula ($ - $$) calcula o tamanho do código que escrevemos até agora, para preencher até o fim
times 510 - ($ - $$) db 0
; preenchendo os últimos dois bytes (511, 512) do bootloader, com a assinatura 0xAA55, para indicar que é o fim do bootloader
; E que se trata de uma assinatura MBR, e que a BIOS pode ler
dw 0xaa55