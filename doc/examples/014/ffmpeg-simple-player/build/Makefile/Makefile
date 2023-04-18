MAKEFILE      = Makefile

EQ            = =

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = 
CFLAGS        = -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC $(DEFINES)
CXXFLAGS      = -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC $(DEFINES)
INCPATH       = -I. $(FF_INC) $(SDL_INC)
QMAKE         = /usr/lib/qt5/bin/qmake
DEL_FILE      = rm -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p
COPY          = cp -f
COPY_FILE     = cp -f
COPY_DIR      = cp -f -R
INSTALL_FILE  = install -m 644 -p
INSTALL_PROGRAM = install -m 755 -p
INSTALL_DIR   = cp -f -R
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
TAR           = tar -cf
COMPRESS      = gzip -9f
DISTNAME      = ffmpeg-simple-player1.0.0
DISTDIR       =./.tmp/ffmpeg-simple-player1.0.0
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = $(FF_LIB) -lavcodec -lavformat -lswscale -lswresample -lavutil $(SDL_LIB) -lSDL2 -lpthread -lz
AR            = ar cqs
RANLIB        = 
SED           = sed
STRIP         = strip

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = ../../src/AudioDecodeThread.cpp \
		../../src/AudioPlay.cpp \
		../../src/DemuxThread.cpp \
		../../src/FFmpegPlayer.cpp \
		../../src/log.cpp \
		../../src/main.cpp \
		../../src/PacketQueue.cpp \
		../../src/RenderView.cpp \
		../../src/SDLApp.cpp \
		../../src/ThreadBase.cpp \
		../../src/Timer.cpp \
		../../src/VideoDecodeThread.cpp 
OBJECTS       = AudioDecodeThread.o \
		AudioPlay.o \
		DemuxThread.o \
		FFmpegPlayer.o \
		log.o \
		main.o \
		PacketQueue.o \
		RenderView.o \
		SDLApp.o \
		ThreadBase.o \
		Timer.o \
		VideoDecodeThread.o
DIST          = \
		../../src/AudioDecodeThread.h \
		../../src/AudioPlay.h \
		../../src/DemuxThread.h \
		../../src/FFmpegPlayer.h \
		../../src/log.h \
		../../src/PacketQueue.h \
		../../src/RenderView.h \
		../../src/SDLApp.h \
		../../src/ThreadBase.h \
		../../src/Timer.h \
		../../src/VideoDecodeThread.h \
		../../src/AudioDecodeThread.cpp \
		../../src/AudioPlay.cpp \
		../../src/DemuxThread.cpp \
		../../src/FFmpegPlayer.cpp \
		../../src/log.cpp \
		../../src/main.cpp \
		../../src/PacketQueue.cpp \
		../../src/RenderView.cpp \
		../../src/SDLApp.cpp \
		../../src/ThreadBase.cpp \
		../../src/Timer.cpp \
		../../src/VideoDecodeThread.cpp
DESTDIR       = 
TARGET        = ffmpeg-simple-player


first: all
####### Build rules

ffmpeg-simple-player:  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

all: Makefile ffmpeg-simple-player

dist: distdir FORCE
	(cd `dirname $(DISTDIR)` && $(TAR) $(DISTNAME).tar $(DISTNAME) && $(COMPRESS) $(DISTNAME).tar) && $(MOVE) `dirname $(DISTDIR)`/$(DISTNAME).tar.gz . && $(DEL_FILE) -r $(DISTDIR)

distdir: FORCE
	@test -d $(DISTDIR) || mkdir -p $(DISTDIR)
	$(COPY_FILE) --parents $(DIST) $(DISTDIR)/
	$(COPY_FILE) --parents ../../src/AudioDecodeThread.h ../../src/AudioPlay.h ../../src/DemuxThread.h ../../src/FFmpegPlayer.h ../../src/log.h ../../src/PacketQueue.h ../../src/RenderView.h ../../src/SDLApp.h ../../src/ThreadBase.h ../../src/Timer.h ../../src/VideoDecodeThread.h $(DISTDIR)/
	$(COPY_FILE) --parents ../../src/AudioDecodeThread.cpp ../../src/AudioPlay.cpp ../../src/DemuxThread.cpp ../../src/FFmpegPlayer.cpp ../../src/log.cpp ../../src/main.cpp ../../src/PacketQueue.cpp ../../src/RenderView.cpp ../../src/SDLApp.cpp ../../src/ThreadBase.cpp ../../src/Timer.cpp ../../src/VideoDecodeThread.cpp $(DISTDIR)/


clean: 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


distclean: clean 
	-$(DEL_FILE) $(TARGET) 

####### Sub-libraries

check: first

####### Compile

AudioDecodeThread.o: ../../src/AudioDecodeThread.cpp ../../src/AudioDecodeThread.h \
		../../src/ThreadBase.h \
		../../src/FFmpegPlayer.h \
		../../src/PacketQueue.h \
		../../src/log.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o AudioDecodeThread.o ../../src/AudioDecodeThread.cpp

AudioPlay.o: ../../src/AudioPlay.cpp ../../src/AudioPlay.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o AudioPlay.o ../../src/AudioPlay.cpp

DemuxThread.o: ../../src/DemuxThread.cpp ../../src/DemuxThread.h \
		../../src/ThreadBase.h \
		../../src/log.h \
		../../src/FFmpegPlayer.h \
		../../src/PacketQueue.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o DemuxThread.o ../../src/DemuxThread.cpp

FFmpegPlayer.o: ../../src/FFmpegPlayer.cpp ../../src/FFmpegPlayer.h \
		../../src/PacketQueue.h \
		../../src/DemuxThread.h \
		../../src/ThreadBase.h \
		../../src/VideoDecodeThread.h \
		../../src/AudioDecodeThread.h \
		../../src/AudioPlay.h \
		../../src/log.h \
		../../src/SDLApp.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o FFmpegPlayer.o ../../src/FFmpegPlayer.cpp

log.o: ../../src/log.cpp ../../src/log.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o log.o ../../src/log.cpp

main.o: ../../src/main.cpp ../../src/RenderView.h \
		../../src/SDLApp.h \
		../../src/Timer.h \
		../../src/log.h \
		../../src/AudioPlay.h \
		../../src/FFmpegPlayer.h \
		../../src/PacketQueue.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o ../../src/main.cpp

PacketQueue.o: ../../src/PacketQueue.cpp ../../src/PacketQueue.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o PacketQueue.o ../../src/PacketQueue.cpp

RenderView.o: ../../src/RenderView.cpp ../../src/RenderView.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o RenderView.o ../../src/RenderView.cpp

SDLApp.o: ../../src/SDLApp.cpp ../../src/SDLApp.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o SDLApp.o ../../src/SDLApp.cpp

ThreadBase.o: ../../src/ThreadBase.cpp ../../src/ThreadBase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o ThreadBase.o ../../src/ThreadBase.cpp

Timer.o: ../../src/Timer.cpp ../../src/Timer.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o Timer.o ../../src/Timer.cpp

VideoDecodeThread.o: ../../src/VideoDecodeThread.cpp ../../src/VideoDecodeThread.h \
		../../src/ThreadBase.h \
		../../src/FFmpegPlayer.h \
		../../src/PacketQueue.h \
		../../src/log.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o VideoDecodeThread.o ../../src/VideoDecodeThread.cpp

####### Install

install:  FORCE

uninstall:  FORCE

FORCE:

