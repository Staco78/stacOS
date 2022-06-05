
IN=ext2.cpp
OUT=../../../initrd/ext2.ko

all:
	x86_64-elf-gcc -c -ffreestanding -Wall -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I../../include -lgcc  -fpermissive -fno-exceptions -fno-rtti $(IN) -o $(OUT) -fno-asynchronous-unwind-tables -Qn
