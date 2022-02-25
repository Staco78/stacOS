GCC=x86_64-elf-gcc
LD=x86_64-elf-ld

GCC_FLAGS=-ffreestanding -Wall -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Isrc/include -lgcc -fpermissive
LD_FLAGS=-z max-page-size=0x1000 -nostdlib 

KERNEL_CPP_SCRS=$(wildcard src/kernel/*.cpp) $(wildcard src/kernel/**/*.cpp)
KERNEL_ASM_SCRS=$(wildcard src/kernel/*.asm)
KERNEL_OBJS=$(KERNEL_ASM_SCRS:.asm=.o) $(KERNEL_CPP_SCRS:.cpp=.o)


all: clean qemu

qemu: myos.iso
	qemu-system-x86_64 -cdrom myos.iso -smp 4 -monitor stdio

bochs: myos.iso
	bochs 

clean:
	rm -rf	 iso 
	rm -f ./**/**/*.o
	rm -f ./**/**/**/*.o
	rm -f myos.bin
	rm -f myos.iso

dir:
	mkdir -p iso/boot/grub

%.o: %.cpp
	$(GCC) -c $< -o $@ $(GCC_FLAGS)

%.o: %.asm
	nasm -felf64 $< -o $@

myos.bin: $(KERNEL_OBJS)
	$(LD) -T linker.ld $^ -o $@ $(LD_FLAGS)

myos.iso: myos.bin dir
	cp myos.bin iso/boot/
	cp grub.cfg iso/boot/grub/
	grub-mkrescue -o $@ iso