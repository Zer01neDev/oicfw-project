release: all
	psp-build-exports -s exports.exp
	bin2c plutonium.prx ../plutonium/plutonium_Driver.c plutonium
	sed -i 's#((aligned(16)))#((aligned(64)))#gi' ../plutonium/plutonium_Driver.c
	sed -i 's#static##gi' ../plutonium/plutonium_Driver.c
#	psp-packer plutonium.prx

TARGET = plutonium
OBJS = main.o

CFLAGS = -Os -G0 -Wall -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

PSP_FW_VERSION = 550

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -lpsppower_driver -lpspsystemctrl_kernel -lpspmodulemgr_kernel

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	rm *.elf
# ../*.o