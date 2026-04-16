[bits 32]
[extern kernel_main] ; Avisa o Assembly que essa função existe em outro arquivo (o C)

call kernel_main     ; Quando a CPU cair aqui, ela chama o C em segurança
jmp $                ; Se o C falhar ou tentar retornar, travamos o PC em segurança