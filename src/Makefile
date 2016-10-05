OBJS =	framedisplay.o \
	mbaa_framedisplay.o \
	mbaa_character.o \
	mbaa_framedata.o \
	mbaa_arx.o \
	mbaa_cg2.o \
	mbaa_tileimage.o \
	mbaa_iso.o \
	mbaacc_framedisplay.o \
	mbaacc_character.o \
	mbaacc_framedata.o \
	mbaacc_cg.o \
	mbaacc_pack.o \
	touhou_framedisplay.o \
	touhou_framedata.o \
	touhou_packfiles.o \
	touhou_images.o \
	abk_framedisplay.o \
	abk_framedata.o \
	abk_packfiles.o \
	abk_images.o \
	ougon_data.o \
	ougon_framedata.o \
	ougon_framedisplay.o \
	texture.o \
	clone.o \
	render.o

LINUX_OBJS = $(OBJS) simple_gui.o
WINDOWS_OBJS = $(OBJS) windows_gui.o
ALL_OBJS = $(OBJS) simple_gui.o windows_gui.o

#CXX = g++
#CFLAGS = -g -Wall -I/usr/include/GL -I/usr/include/SDL -I/home/mauve/dev/fdisp/src/include
#LIBS = -lpng -lGL -lSDL
#LDFLAGS = -g
#PROJECT = framedisplay

CXX = i586-mingw32msvc-g++
CFLAGS = -s -Os -Wall -I/usr/i586-mingw32msvc/include/GL -I/usr/i586-mingw32msvc/include/SDL -I/home/mauve/dev/fdisp/src/include
LIBS = /usr/i586-mingw32msvc/lib/libpng12.a -lz -lopengl32 -lcomctl32 -lole32
LDFLAGS = -mwindows
PROJECT = framedisplay.exe

all: $(PROJECT)

clean:
	rm -f $(ALL_OBJS) framedisplay framedisplay.exe

.cpp.o:
	$(CXX) $(CFLAGS) -o $@ -c $<

framedisplay: $(SUBDIRS) $(LINUX_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(LINUX_OBJS) $(LIBS)

framedisplay.exe: $(SUBDIRS) $(WINDOWS_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(WINDOWS_OBJS) $(LIBS)

imageset: $(OBJS) imageset.o
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) imageset.o $(LIBS)
