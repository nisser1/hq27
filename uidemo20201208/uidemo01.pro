# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Add-in.
# ------------------------------------------------------

TEMPLATE = app
TARGET = display_bit
DESTDIR = ../bin
QT += network widgets
CONFIG += debug console
DEFINES += _WINDOWS QT_WIDGETS_LIB QT_NETWORK_LIB
INCLUDEPATH += ./GeneratedFiles/Debug \
    ./GeneratedFiles \
    . \
    ./temp/moc \
    ./temp/ui
LIBS += -L/home/ht706/nisser/lib/ -luart245
DEPENDPATH +=
MOC_DIR += ./GeneratedFiles/debug
OBJECTS_DIR += debug
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles
INCLUDEPATH +=  /home/ht706/nisser/lib/
include(uidemo01.pri)

HEADERS +=

SOURCES +=