# ArzaOS 
Um Sistema Operacional partindo do zero, que visa chegar em uma arquitetura mais completa, em 64 bits.

Atualmente, Arza é apenas um Kernel com inttegração VGA de 32 bits, com um bootloader próprio.

Este projeto visa documentar a jornada de construção de um núcleo (kernel) funcional, explorando os níveis mais baixos da arquitetura x86. A ideia do projeto é compreender como o software se comunica diretamente com o hardware, e como essas estruturas se correlacionam e conectam do zero.

O projeto eestá sendo desenvolvido para uso próprio e com propósito educacional. No momento o projeto ainda está na base, somente com o Kernel, e todo em português - BR

## 🚀 Funcionalidades Atuais
- **Bootloader Customizado:** Salto do modo real de 16 bits para o modo protegido de 32 bits.
- **Gestão de Interrupções:** Implementação completa da IDT (Interrupt Descriptor Table) e ISRs (Interrupt Service Routines).
- **Driver de Vídeo VGA:** Suporte para escrita no ecrã com cores, cursor dinâmico pelo I/O e scroll automático.
- **Arquitetura Modular:** Separação clara entre código de CPU, Drivers, Kernel e uma pequena LibC.

## 🛠️ Como Compilar
Para buildar a base do Kernel do ArzaOS, é necessário a utilização das ferramentas `nasm`, `gcc` (com suporte para 32 bits), `ld` e o emulador `qemu`.

1. Com tudo configurado, você pode simplesmente rodar as configurações do `Makefile`:

```bash
make run
```

2. Se deseja limpar a construção dos binários, basta utilizar:
```bash
make clean
```