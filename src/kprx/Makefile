TARGET = kprx
OBJS = main.o libpsardumper.o pspdecrypt.o

INCDIR = ../include
CFLAGS = -Os -G0
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = kprx.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = ../lib
LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -lpsphen -lpspnand_driver -lpspumdman_driver -lpspwlan -lpspnet_adhoc -lpspnet_adhocctl -lpspnet -lpsputility -lpspsemaphore -lpspmesgd_driver -lpspnwman_driver -lpsputilsforkernel -lpspsystemctrl_kernel -lpspkernel

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
