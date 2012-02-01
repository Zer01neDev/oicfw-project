release: all
	pack-pbp EBOOT.PBP PARAM_UPDATER.SFO ICON0.PNG NULL NULL NULL NULL plutonium_updater.prx NULL
	rm *.elf *.prx *.o

TARGET = plutonium_updater
OBJS = main.o ../Plutonium_Driver/Plutonium.o plutonium_Driver.o

CFLAGS = -O2 -G0 -Wall -DPSPFW5xx
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

#EXTRA_TARGETS = EBOOT.PBP
PSP_FW_VERSION = 550
BUILD_PRX = 1

LIBS = -lpspvshbridge

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
