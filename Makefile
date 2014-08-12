BDIR=bin
CDIR=src
ODIR=obj
MDIR=media
LDIR=maps

OUT=cyborg
DAT=content.dat
DAT_H=$(CDIR)/media.h

CC=cc
STRIP=strip
CFLAGS=-Wall -O2 -g
LDFLAGS=-lSDL -lSDL_image -lGL

WIN32_EXE=$(OUT).exe
WIN32_ZIP=cyborg_win32.zip
WIN32_FOLDER=cyborg-win32
WIN32_CC=/opt/mingw32/bin/i686-w64-mingw32-gcc
WIN32_CFLAGS=-I"dependencies/SDL-1.2.15/include/"
WIN32_LDFLAGS=-L"dependencies/SDL-1.2.15/lib/"
WIN32_LDFLAGS+= -L"dependencies/SDL_image-1.2.12/lib/x86/"
WIN32_LDFLAGS+= -lmingw32 -lSDLmain -lSDL_image -lSDL -lopengl32

SRC=$(wildcard $(CDIR)/*.c)
OBJ=$(SRC:$(CDIR)/%.c=$(ODIR)/%.o)
DEP=$(OBJ:%.o=%.d)

MEDIAS=$(wildcard $(MDIR)/*)
PACKMEDIA=python packmedia.py

CMAP=python mapcompile.py
MAPSIN=$(wildcard $(LDIR)/*.txt)
MAPSOUT=$(MAPSIN:$(LDIR)/%.txt=$(MDIR)/%.map)

.PHONY: all conf win32 clean re

all: $(BDIR) $(ODIR) $(DAT) $(OUT)

-include $(DEP)

$(BDIR):
	mkdir $@

$(ODIR):
	mkdir $@

$(OUT): $(OBJ) $(DAT)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)
	$(STRIP) $@

$(ODIR)/%.o: $(CDIR)/%.c
	$(CC) -o $@ -c $< -MMD -MF $(@:%.o=%.d) $(CFLAGS)

$(DAT): $(MEDIAS) $(MAPSOUT)
	$(PACKMEDIA) $(MDIR) $(DAT) $(DAT_H)

$(MDIR)/%.map: $(LDIR)/%.txt
	$(CMAP) $< $@

conf:
	mkdir $(BDIR) $(ODIR)

$(WIN32_EXE): $(SRC) $(DAT)
	$(WIN32_CC) -o $(WIN32_EXE) $(SRC) $(WIN32_CFLAGS) $(WIN32_LDFLAGS)

$(WIN32_ZIP): $(WIN32_EXE)
	rm -rf /tmp/$(WIN32_FOLDER)
	mkdir /tmp/$(WIN32_FOLDER)
	cp $< $(DAT) dependencies/*.dll /tmp/$(WIN32_FOLDER)
	(cd /tmp && zip -r $@ $(WIN32_FOLDER))
	cp /tmp/$@ .

win32: $(WIN32_EXE) $(WIN32_ZIP)

clean:
	$(RM) $(OUT) $(WIN32_EXE) $(WIN32_ZIP) $(DAT) $(DAT_H) -r $(BDIR) $(ODIR) $(MAPSOUT)

re: clean all

#avconv -y -i media/intro.wav -ar 8000 -ac 1 /tmp/o.wav

