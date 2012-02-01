release: all
#	pack-pbp EBOOT.PBP PARAM_UPDATER.SFO ICON0.PNG NULL NULL NULL NULL plutonium_updater.prx NULL
	rm *.elf *.prx *.o

TARGET = plutonium_updater
OBJS = main.o Plutonium_Driver.o Plutonium.o pspDegeneration_Driver.o pspDegeneration.o

CFLAGS = -O2 -G0 -Wall -DPSPFW5xx
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_ICON = ICON0.PNG
PSP_EBOOT_SFO = PARAM_UPDATER.SFO

PSP_FW_VERSION = 550

BUILD_PRX = 1

LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -lpsppower -lpspsystemctrl_user -lpspvshbridge

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
