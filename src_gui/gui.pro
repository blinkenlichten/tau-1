QT += charts
CONFIG += c++11

TARGET = display_gui
LIBS += ../external/lib/libt1net.a -L../external/lib -lmlm -lczmq -lzmq

INCLUDEPATH += ../include ../external/include

HEADERS += \
    display_gui/simplexy.h

SOURCES += \
    display_gui/simplexy.cpp display_gui/main.cpp \

