TARGET = nge2
OBJS = 	test.o
BUILD_PRX = 1
PSP_FW_VERSION=371  
INCDIR = ../../include
LIBDIR = ../../lib
CFLAGS = -G0 -w -O2 -D_DEBUG_LOG
CXXFLAGS = $(CFLAGS) -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS =
LIBS +=  -lnge2 -lfreetype -lpng -lungif -lz -ljpeg -lg -lstdc++  -lpspmpeg -lpspaudiocodec -lpspaudio -lpsppower  -lpspgum -lpspgu -lpsprtc -lm   

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = demo7

PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
include $(PSPSDK)/lib/build.mak

