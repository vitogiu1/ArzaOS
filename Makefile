ASM = nasm
CC = gcc
LD = ld
EMU = qemu-system-x86_64

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector
LDFLAGS = -m elf_i386 -Ttext 0x9000 --oformat binary

# Procura todos os arquivos por meio do wildcard
C_SOURCES = $(wildcard kernel/*.c drivers/*.c libc/*.c cpu/*.c)

# Pega a lista de .c e troca a extensão para .o
OBJ = ${C_SOURCES:.c=.o}

# ALVOS PRINCIPAIS
all: os-image.bin

run: os-image.bin
	$(EMU) -drive format=raw,file=os-image.bin

# ------ Construção ---------

# 1. Imagem do Disco
os-image.bin: boot/boot.bin kernel.bin
	@echo "Construindo o disco do ArzaOS..."
	dd if=/dev/zero of=os-image.bin bs=512 count=2880 >/dev/null 2>&1
	dd if=boot/boot.bin of=os-image.bin conv=notrunc >/dev/null 2>&1
	dd if=kernel.bin of=os-image.bin bs=512 seek=1 conv=notrunc >/dev/null 2>&1
	@echo "Imagem construida com sucesso!"

# 2. Linkagem do Kernel
# O $^ significa "pegue todas as dependências desta regra"
# O boot/kernel_entry.o TEM que ser o primeiro da lista!
kernel.bin: boot/kernel_entry.o cpu/interrupt.o ${OBJ}
	$(LD) $(LDFLAGS) -e kernel_main -o kernel.bin $^

# 3. Regra genérica: Como transformar QUALQUER .c em .o
# O $< é o arquivo .c, o $@ é o arquivo .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 4. Como compilar os Assemblys que estão na pasta boot
boot/%.o: boot/%.asm
	$(ASM) -f elf32 $< -o $@

cpu/%.o: cpu/%.asm
	$(ASM) -f elf32 $< -o $@

boot/boot.bin: boot/boot.asm
	$(ASM) -f bin $< -o $@

# Clean básico
clean:
	rm -f *.bin
	rm -f boot/*.bin boot/*.o kernel/*.o drivers/*.o libc/*.o cpu/*.o