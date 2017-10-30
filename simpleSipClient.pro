QT += core
QT += widgets
QT -= gui
QT += network websockets

CONFIG += c++11

TARGET = sipuac
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

#工程中包含的头文件
HEADERS += mediaclient.h \
    defines.h \
    rtprecver.h \
    pstoh264.h \
    frameresolve/sms.h \
    frameresolve/access.h \
    decoder/decoder.h \
    imagelabel.h

SOURCES += main.cpp \
    mediaclient.cpp \
    rtprecver.cpp \
    pstoh264.cpp \
    frameresolve/sms.cpp \
    frameresolve/access.cpp \
    decoder/decoder.cpp \
    imagelabel.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DESTDIR += ./bin
#MOC_DIR += ./tmp
#OBJECTS_DIR += ./tmp

unix {
    INCLUDEPATH += $$PWD/pjlibs/pjlib-util/include
    INCLUDEPATH += $$PWD/pjlibs/pjmedia/include
    INCLUDEPATH += $$PWD/pjlibs/pjsip/include
    INCLUDEPATH += $$PWD/pjlibs/pjnath/include

    INCLUDEPATH += $$PWD/jrtplib/include
    #ffmpeg
    INCLUDEPATH += $$/usr/local/ffmpeg/include

    LIBS += -lm -lpthread -lcrypto -lssl -lasound

    LIBS += $$PWD/pjlibs/pjsip/lib/libpjsua-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjsip/lib/libpjsip-ua-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjsip/lib/libpjsua2-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjmedia/lib/libpjmedia-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjmedia/lib/libpjsdp-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjsip/lib/libpjsip-simple-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjsip/lib/libpjsip-ua-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjsip/lib/libpjsip-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjmedia/lib/libpjmedia-videodev-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjmedia/lib/libpjsdp-x86_64-unknown-linux-gnu.a

    LIBS += $$PWD/pjlibs/pjmedia/lib/libpjmedia-codec-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjmedia/lib/libpjmedia-audiodev-x86_64-unknown-linux-gnu.a

    LIBS += $$PWD/pjlibs/pjmedia/lib/libpjmedia-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjnath/lib/libpjnath-x86_64-unknown-linux-gnu.a

    LIBS += $$PWD/pjlibs/third_party/lib/libwebrtc-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/third_party/lib/libyuv-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/third_party/lib/libg7221codec-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/third_party/lib/libgsmcodec-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/third_party/lib/libresample-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/third_party/lib/libilbccodec-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/third_party/lib/libspeex-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/third_party/lib/libsrtp-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjlib-util/lib/libpjlib-util-x86_64-unknown-linux-gnu.a
    LIBS += $$PWD/pjlibs/pjlib/lib/libpj-x86_64-unknown-linux-gnu.a

    LIBS += $$PWD/jrtplib/lib/libjrtp.a

    LIBS += -lm -lnsl -lrt -lpthread -lasound -lcrypto -lssl

    LIBS += -L/usr/local/ffmpeg/lib/ -lavcodec -lavfilter -lavutil -lavdevice -lavformat -lswscale -lswresample

message("unix")
}

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
