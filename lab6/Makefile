SRC_DIR = src
OBJ_DIR = obj
INC_DIR = inc

SRCS = $(wildcard $(SRC_DIR)/*.c)
#OBJS = $(SRCS:.c=.o)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
CFLAGS = -Wall -O0 -ffreestanding -nostdinc -nostdlib -nostartfiles -I$(INC_DIR) -g

all: clean kernel8.img

start.o: start.S
	aarch64-linux-gnu-gcc $(CFLAGS) -c start.S -o start.o
exception.o: exception.S
	aarch64-linux-gnu-gcc $(CFLAGS) -c exception.S -o exception.o
context.o : context.S
	aarch64-linux-gnu-gcc $(CFLAGS) -c context.S -o context.o
vm.o : vm.S
	aarch64-linux-gnu-gcc $(CFLAGS) -c vm.S -o vm.o

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	aarch64-linux-gnu-gcc $(CFLAGS) -c $< -o $@

kernel8.img: exception.o start.o context.o vm.o $(OBJS)
	aarch64-linux-gnu-ld -g -nostdlib -nostartfiles start.o exception.o context.o vm.o $(OBJS) -T link.ld -o kernel8.elf
	aarch64-linux-gnu-objcopy -g -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf $(OBJ_DIR)/*.o >/dev/null 2>/dev/null || true

run:
#	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

port:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial pty

gdb:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -S -s -serial null -serial stdio

int:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -d int
