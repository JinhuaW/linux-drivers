obj-m := globalfifo.o test_irq.o

BUILDROOT_OUTPUT=$(BUILDROOT)/output

OPTIONS= -C $(LINUX_DIR) M=$(PWD) ARCH=$(KERNEL_ARCH) CROSS_COMPILE=$(TARGET_CROSS)

all:
	make $(OPTIONS) modules
clean:
	make $(OPTIONS) clean
install:
	make $(OPTIONS) INSTALL_MOD_PATH=$(BILDROOT_OUTPUT)/target INSTALL_MOD_STRIP=1 INSTALL_MOD_DIR=spe modules_install
headers_install:
	@echo "installing headers"
