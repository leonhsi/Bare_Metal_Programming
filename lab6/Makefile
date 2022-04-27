SRC_DIR = src
OBJ_DIR = obj
INC_DIR = inc

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
CFLAGS_KERNEL = -Wall -O0 -ffreestanding -nostdinc -nostdlib -nostartfiles -I$(INC_DIR) -g

# USER_SRC_DIR = user/src
# USER_OBJ_DIR = user/obj
# USER_INC_DIR = user/inc

# USER_SRCS = $(wildcard $(USER_SRC_DIR)/*.c)
# USER_OBJS = $(patsubst $(USER_SRC_DIR)/%.c, $(USER_OBJ_DIR)/%.o, $(USER_SRCS))
# CFLAGS_USER = -Wall -O0 -ffreestanding -nostdinc -nostdlib -nostartfiles -I$(USER_INC_DIR) -g

CPIO_DIR = rootfs

.PHONY: kernel8.img 

all: clean kernel8.img 

# build kernel #
boot.o: boot.S
	aarch64-linux-gnu-gcc $(CFLAGS_KERNEL) -c boot.S -o boot.o
exception.o: exception.S
	aarch64-linux-gnu-gcc $(CFLAGS_KERNEL) -c exception.S -o exception.o
context.o : context.S
	aarch64-linux-gnu-gcc $(CFLAGS_KERNEL) -c context.S -o context.o

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	aarch64-linux-gnu-gcc $(CFLAGS_KERNEL) -c $< -o $@

kernel8.img: exception.o boot.o context.o $(OBJS)
	aarch64-linux-gnu-ld -g -nostdlib -nostartfiles boot.o exception.o context.o $(OBJS) -T link.ld -o kernel8.elf
	aarch64-linux-gnu-objcopy -g -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf boot.o exception.o context.o $(OBJ_DIR)/*.o  >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb
#	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

port:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial pty

gdb:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -S -s -serial null -serial stdio

int:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -d int

load_user:
	cp user/user.img rootfs/ && \
	cd $(CPIO_DIR) && \
	find . | cpio -o -H newc > ../initramfs.cpio && \
	cd $(UPPER_LEVEL)

