obj-m := globalfifo.o
PWD :=$(shell pwd)

KDIR := /home/jinhuawu/Data/buildroot_vexpress/output/build/linux-c787c02e1d83382591c427319f384175158e5d37

all:
	make -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=/home/jinhuawu/Data/buildroot_vexpress/output/host/usr/bin/arm-buildroot-linux-gnueabi- modules
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers
