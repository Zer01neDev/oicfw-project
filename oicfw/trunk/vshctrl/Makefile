release: all
	psp-packer vshctrl.prx

TARGET = vshctrl
OBJS = main.o vshctrl.o virtualpbpmgr.o

CFLAGS = -Os -G0 -Wall -fno-pic -fshort-wchar -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

PSP_FW_VERSION = 500

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -lpsppower_driver -lpsploadexec_kernel -lpspsystemctrl_kernel -lpspreg_driver -lpsprtc_driver\
	-lpspusbdevice_driver

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm *.elf
# ../*.o