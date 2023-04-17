QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

DESTDIR = $$PWD/../bin/

INCLUDEPATH += $$PWD/../third-party/SDL2-2.0.22/include
INCLUDEPATH += $$PWD/../third-party/ffmpeg-5.1.0-dev/include

LIBS += $$PWD/../third-party/ffmpeg-5.1.0-dev/lib/avutil.lib
LIBS += $$PWD/../third-party/ffmpeg-5.1.0-dev/lib/avcodec.lib
LIBS += $$PWD/../third-party/ffmpeg-5.1.0-dev/lib/avformat.lib
LIBS += $$PWD/../third-party/ffmpeg-5.1.0-dev/lib/swresample.lib
LIBS += $$PWD/../third-party/ffmpeg-5.1.0-dev/lib/swscale.lib
LIBS += $$PWD/../third-party/ffmpeg-5.1.0-dev/lib/avdevice.lib
LIBS += $$PWD/../third-party/SDL2-2.0.22/lib/x64/SDL2.lib

DEFINES += SDL_MAIN_HANDLED

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AudioDecodeThread.cpp \
    AudioPlay.cpp \
    DemuxThread.cpp \
    FFmpegPlayer.cpp \
    PacketQueue.cpp \
    RenderView.cpp \
    SDLApp.cpp \
    ThreadBase.cpp \
    Timer.cpp \
    VideoDecodeThread.cpp \
    log.cpp \
    main.cpp

HEADERS += \
    AudioDecodeThread.h \
    AudioPlay.h \
    DemuxThread.h \
    FFmpegPlayer.h \
    PacketQueue.h \
    RenderView.h \
    SDLApp.h \
    ThreadBase.h \
    Timer.h \
    VideoDecodeThread.h \
    log.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
