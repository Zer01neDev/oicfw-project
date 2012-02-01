release: all
	psp-packer recovery.prx

TARGET = recovery
OBJS = main.o menu.o mydebug.o vshbridge.o

CFLAGS = -O2 -G0 -Wall -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

PSP_FW_VERSION = 550
BUILD_PRX = 1
#PRX_EXPORTS = exports.exp

USE_PSPSDK_LIBC = 1
USE_USER_LIBS = 1

LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -lpspusb -lpspusbstor -lpspsystemctrl_user -lpspreg -lpspusbdevice -lpspvshbridge

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
