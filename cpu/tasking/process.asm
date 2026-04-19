[bits 32]
section .text

; Função que irá ler o EIP
global read_eip
read_eip:
    ; Quando o C chamar esta função, o processador guardará a linha de retorno na pilha
    ; Depois, basta devolver esta linha da pilha ao EAX
    pop eax
    jmp eax

; O switch task
global switch_task
switch_task:
    cli

    mov ecx, [esp+4]    ; Pega o EIP da nova task
    mov eax, [esp+8]    ; Pega a pilha (ESP) da nova task
    mov edx, [esp+12]   ; Pega a base da Pilha (EBP) da nova task
    mov ebx, [esp+16]   ; Pega a Memóriva Virtual (CR3) do Novo Programa

    ; Troca para a nova memória virtual da nova task
    mov cr3, ebx

    ; Troca a a nova pilha
    mov esp, eax
    mov ebp, edx

    ; Injetar o 0x12345 no EAX
    ; Quando o programa acordar novamente no C, ele entenderá que a read_eip devolveu 0x12345
    mov eax, 0x12345

    ; Devolve ao novo programa
    sti
    jmp ecx