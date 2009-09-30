PSPSDK=$(shell psp-config --pspsdk-path)
PSPDIR=$(shell psp-config --psp-prefix)
TARGET_LIB = libnge2.a
OBJS = helper/iniparser.o helper/dictionary.o \
       libnge2.o nge_charsets.o nge_debug_log.o nge_timer.o nge_sys_compatible.o \
       nge_font_hzk.o nge_font_nfont.o nge_image_load.o  nge_font.o nge_gif_animation.o \
       nge_io_file.o nge_io_dir.o nge_misc.o nge_rle.o nge_main.o nge_gif_show.o \
       nge_graphics_psp.o nge_graphics_ext.o nge_input_proc.o nge_font_freetype.o \
       nge_vram.o audiosystem/libcoolaudio.o audiosystem/play_ogg_psp.o  \
       audiosystem/play_mp3_psp.o audiosystem/play_pcm_psp.o movieplay/nge_movie.o \
       net/event.o net/log.o net/select.o hge/hgecolor.o hge/hgeparticle.o hge/hgepmanager.o \
       hge/hgerect.o hge/hgevector.o effect/nge_image_effect.o
INCDIR = psp/include psp/include/freetype2 ../include
LIBDIR = psp/lib 
CFLAGS = -G0 -w -DNGE2_NET -D_DEBUG_LOG -D_DEBUG_STDOUT
LIBS += -ljpeg -lpng -lz
CXXFLAGS = $(CFLAGS) -fno-rtti 
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION=371
PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
include $(PSPSDK)/lib/build.mak
install:$(TARGET_LIB)
	rm -rf ../lib/libnge2.a
	cp libnge2.a ../lib/
	cp helper/*.h ../include/helper/
	cp audiosystem/interface.h ../include/audiosystem/
	cp movieplay/nge_movie.h ../include/movieplay/
	cp net/event.h ../include/net/
	cp net/nge_net_common.h ../include/net/
	cp effect/nge_image_effect.h ../include/effect/

	