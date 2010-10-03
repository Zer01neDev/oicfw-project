release: all
	pack-pbp EBOOT.PBP PARAM_UPDATER.SFO ICON0.PNG NULL NULL NULL NULL OpenIdeaUpdater.prx NULL
	rm *.elf *.prx *.o

TARGET = OpenIdeaUpdater
OBJS = main.o ../Nuclear_Driver/Nuclear.o Nuclear_Driver.o vsh.o

CFLAGS = -O2 -G0 -Wall -DPSPFW5xx
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

PSP_FW_VERSION = 550
BUILD_PRX = 1

LIBS = -lpspvshbridge

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
