#############################################################################
# Makefile for building: out/photon
# Project:  photon
#############################################################################

MAKEFILE      = Makefile

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = 
CFLAGS        = -m64 -pipe -O2 -D_REENTRANT -Wall -W -fPIE $(DEFINES)
CXXFLAGS      = -m64 -pipe -O2 -D_REENTRANT -Wall -W -fPIE $(DEFINES)
INCPATH       = -I.
LINK          = g++
LFLAGS        = -m64 -Wl,-O1
LIBS          = $(SUBLIBS) -L. -L/usr/X11R6/lib64 -L/usr/lib/x86_64-linux-gnu -lwfdb -lpthread
AR            = ar cqs
RANLIB        = 
QMAKE         = 
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = cp -f
COPY_DIR      = cp -f -R
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = tmp/release/

####### Files

SOURCES       = main.cpp \
		analyze.cpp \
		EcgAnalyse.cpp

OBJECTS       = tmp/release/main.o \
		tmp/release/analyze.o \
		tmp/release/EcgAnalyse.o \

QMAKE_TARGET  = photon

DESTDIR       = out/#avoid trailing-slash linebreak

TARGET        = out/photon


first: all

test: all
	./test-analyzer.sh ~/proj/physionet.org/physiobank/database/mitdb
	./test-analyzer.sh ~/proj/physionet.org/physiobank/database/nstdb

####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	@test -d out/ || mkdir -p out/
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

folders :
	@mkdir -p tmp/release tmp/debug

qmake_all: FORCE

clean:
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) out/$(TARGET) 


version.h: FORCE
	./subwcrev.sh version.tmpl version.h


####### Compile

tmp/release/main.o: folders main.cpp analyze.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tmp/release/main.o main.cpp

tmp/release/analyze.o: folders analyze.cpp analyze.h \
		EcgData.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tmp/release/analyze.o analyze.cpp

tmp/release/EcgAnalyse.o: folders EcgAnalyse.cpp EcgData.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tmp/release/EcgAnalyse.o EcgAnalyse.cpp


####### Install

install:   FORCE

uninstall:   FORCE

FORCE:

