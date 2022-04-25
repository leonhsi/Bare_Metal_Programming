SRC_DIR = src
OBJ_DIR = obj
INC_DIR = inc

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
CFLAGS_KERNEL = -Wall -O0 -ffreestanding -nostdinc -nostdlib -nostartfiles -I$(INC_DIR) -g

USER_SRC_DIR = user
USER_OBJ_DIR = user/obj

USER_SRCS = $(wildcard $(USER_SRC_DIR)/*.c)
USER_OBJS = $(patsubst $(USER_SRC_DIR)/%.c, $(USER_OBJ_DIR)/%.o, $(USER_SRCS))
CFLAGS_USER = -Wall -O0 -ffreestanding -nostdinc -nostdlib -nostartfiles -g

CPIO_DIR = rootfs
UPPER_LEVEL = ..

.PHONY: kernel8.img user.img

all: clean kernel8.img user.img

# build kernel #
start.o: start.S
	aarch64-linux-gnu-gcc $(CFLAGS_KERNEL) -c start.S -o start.o
exception.o: exception.S
	aarch64-linux-gnu-gcc $(CFLAGS_KERNEL) -c exception.S -o exception.o
context.o : context.S
	aarch64-linux-gnu-gcc $(CFLAGS_KERNEL) -c context.S -o context.o
vm.o : vm.S
	aarch64-linux-gnu-gcc $(CFLAGS_KERNEL) -c vm.S -o vm.o

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	aarch64-linux-gnu-gcc $(CFLAGS_KERNEL) -c $< -o $@

$(USER_OBJ_DIR)/%.o: $(USER_SRC_DIR)/%.c
	aarch64-linux-gnu-gcc $(CFLAGS_USER) -c $< -o $@

kernel8.img: exception.o start.o context.o vm.o $(OBJS)
	aarch64-linux-gnu-ld -g -nostdlib -nostartfiles start.o exception.o context.o vm.o $(OBJS) -T link.ld -o kernel8.elf
	aarch64-linux-gnu-objcopy -g -O binary kernel8.elf kernel8.img

# build user #
user_start.o: $(USER_SRC_DIR)/user_start.S
	aarch64-linux-gnu-gcc $(CFLAGS_KERNEL) -c $< -o $(USER_OBJ_DIR)/$@

user.img: user_start.o $(USER_OBJS)
	aarch64-linux-gnu-ld -g -nostdlib -nostartfiles $(USER_OBJ_DIR)/user_start.o $(USER_OBJS) -T link.ld -o $(USER_SRC_DIR)/user.elf
	aarch64-linux-gnu-objcopy -g -O binary $(USER_SRC_DIR)/user.elf $(USER_SRC_DIR)/user.img

clean:
	rm kernel8.elf user.elf user.img $(OBJ_DIR)/*.o $(USER_OBJ_DIR)/*.o >/dev/null 2>/dev/null || true

run:
#	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

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

