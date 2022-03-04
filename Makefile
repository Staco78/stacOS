GCC=x86_64-elf-gcc
LD=x86_64-elf-ld

C_FLAGS=-ffreestanding -Wall -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Isrc/include -lgcc -g
GCC_FLAGS=$(C_FLAGS) -fpermissive
LD_FLAGS=-z max-page-size=0x1000 -nostdlib -g

KERNEL_C_SCRS=$(shell find ./src/ -name *.c)
KERNEL_CPP_SCRS=$(shell find ./src/ -name *.cpp)
KERNEL_ASM_SCRS=$(shell find ./src/ -name *.asm)
KERNEL_OBJS=$(KERNEL_ASM_SCRS:.asm=.o) $(KERNEL_CPP_SCRS:.cpp=.o) ${KERNEL_C_SCRS:.c=.o}


QEMU_FLAGS=-cdrom myos.iso -smp 4 -cpu max,+pdpe1gb -m 32 -monitor stdio

all: qemu

build: myos.iso

qemu: myos.iso
	qemu-system-x86_64 $(QEMU_FLAGS)

debug: myos.iso
	qemu-system-x86_64 -s -S -boot d $(QEMU_FLAGS)

bochs: myos.iso
	bochs 

clean:
	rm -rf	 iso 
	find ./ -name *.o -delete
	rm -f myos.bin
	rm -f myos.iso

dir:
	mkdir -p iso/boot/grub

%.o: %.cpp
	$(GCC) -c $< -o $@ $(GCC_FLAGS)

%.o: %.c
	$(GCC) -c $< -o $@ $(C_FLAGS)

%.o: %.asm
	nasm -felf64 $< -o $@

myos.bin: $(KERNEL_OBJS)
	$(LD) -T linker.ld $^ -o $@ $(LD_FLAGS)

myos.iso: myos.bin dir
	cp myos.bin iso/boot/
	cp grub.cfg iso/boot/grub/
	grub-mkrescue -o $@ iso