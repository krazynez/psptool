TARGET = main
OBJS = Kprx.o main.o common.o #crt0.o

INCDIR = include
CFLAGS = -Os -G0
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

LIBDIR = lib
LDFLAGS += -nostdlib
LIBS = -lpspusb -lpspusbstor -lpspusbdevice -lpsppower -lz -lpspreg -lpspwlan -lpspopenpsid -lpsprtc -lvlfgui -lvlfgu -lvlfutils -lvlflibc -lpsphen

BUILD_PRX = 1

PSP_FW_VERSION = 271

PSP_EBOOT_TITLE = PSP Tool
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_PSAR = data/files.zip
PSP_EBOOT_ICON = data/images/ICON0.png


PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

clean:
	-rm -rf *.S *.o *.elf *.prx EBOOT* PARA*
