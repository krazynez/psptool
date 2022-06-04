TARGET = main
OBJS = Kprx.o common.o crt0.o main.o

INCDIR = include
CFLAGS = -Wall -Os -G0
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

LIBDIR = lib
LDFLAGS += -nostdlib
LIBS = -lpspusb -lpspusbstor -lpspusbdevice -lpsppower -lz -lpspreg -lpspwlan -lpspopenpsid -lpsprtc -lvlfgui -lvlfgu -lvlfutils -lvlflibc -lpsphen

BUILD_PRX = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
