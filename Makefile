ASM = nasm
CC = gcc
LD = ld
EMU = qemu-system-x86_64

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector
LDFLAGS = -m elf_i386 -Ttext 0x1000 --oformat binary

BUILD_DIR = build

C_SOURCES = $(wildcard kernel/*.c kernel/memory/*.c cpu/tasking/*.c drivers/*.c libc/*.c cpu/*.c)

OBJ = $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SOURCES))

# Alvos Principais
all: $(BUILD_DIR)/os-image.bin

run: $(BUILD_DIR)/os-image.bin
	$(EMU) -drive format=raw,file=$(BUILD_DIR)/os-image.bin

# Regras de construção
$(BUILD_DIR)/os-image.bin: $(BUILD_DIR)/boot/boot_sect.bin $(BUILD_DIR)/boot/stage2.bin $(BUILD_DIR)/kernel.bin
	@echo "Construindo o disco do ArzaOS..."
	@mkdir -p $(dir $@)
	dd if=$(BUILD_DIR)/boot/boot_sect.bin of=$@ conv=notrunc >/dev/null 2>&1
	dd if=$(BUILD_DIR)/boot/stage2.bin of=$@ bs=512 seek=1 conv=notrunc >/dev/null 2>&1
	dd if=$(BUILD_DIR)/kernel.bin of=$@ bs=512 seek=4 conv=notrunc >/dev/null 2>&1
	@echo "Imagem construida com sucesso!"

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/boot/kernel_entry.o $(BUILD_DIR)/cpu/interrupt.o $(BUILD_DIR)/cpu/tasking/process.o ${OBJ}
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -e kernel_main -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf32 $< -o $@

# Regra específica para o bootloader que precisa ser flat binary (.bin) e não ELF (.o)
$(BUILD_DIR)/boot/boot_sect.bin: boot/boot_sect.asm
	@mkdir -p $(dir $@)
	$(ASM) -f bin $< -o $@

$(BUILD_DIR)/boot/stage2.bin: boot/stage2.asm
	@mkdir -p $(dir $@)
	$(ASM) -f bin $< -o $@

clean:
	rm -rf $(BUILD_DIR)