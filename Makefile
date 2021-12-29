obj-m := globalfifo.o test_irq.o misc_test.o platform_dev.o cma_test.o cma_dts.o

BUILDROOT_OUTPUT=$(BUILDROOT)/output

OPTIONS= -C $(LINUX_DIR) M=$(PWD) ARCH=$(KERNEL_ARCH) CROSS_COMPILE=$(TARGET_CROSS)

all:
	make $(OPTIONS) modules
clean:
	make $(OPTIONS) clean
install:
	make $(OPTIONS) INSTALL_MOD_PATH=$(BILDROOT_OUTPUT)/target INSTALL_MOD_STRIP=1 INSTALL_MOD_DIR=spe modules_install
headers_install:
	install -D -m 0644 export/misc_test_ioctl.h $(DESTDIR)/usr/include/linux
