TARGET = kprx
OBJS = main.o libpsardumper.o pspdecrypt.o

INCDIR = ../include
CFLAGS = -Os -G0
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

PSP_FW_VERSION = 271

BUILD_PRX = 1
PRX_EXPORTS = kprx.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = ../lib
LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -lpsphen -lpspnand_driver -lpspwlan -lpspnet_adhoc -lpspnet_adhocctl -lpspnet -lpsputility -lpspsemaphore -lpspmesgd_driver -lpspnwman_driver -lpsputilsforkernel -lpspsystemctrl_kernel -lpspkernel -lpspsysmem_kernel

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	psp-build-exports -s $(PRX_EXPORTS)
	cp Kprx.S ../
	bin2c kprx.prx kprx.h kprx

clean:
	-rm -rf *.S *.o *.elf *.prx kprx.h
