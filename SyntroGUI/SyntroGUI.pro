# This file is part of Syntro
#
# Copyright (c) 2012 Pansenti, LLC. All rights reserved.
#

TEMPLATE = lib
TARGET = SyntroGUI

win32* {
	DESTDIR = Release
}

include(../version.pri)

# common mac or linux
unix {
        target.path = /usr/lib
	headerfiles.path = /usr/include/syntro
	headerfiles.files += *.h
	INSTALLS += headerfiles target
}

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += shared

# No debug in release builds
unix:QMAKE_CXXFLAGS_RELEASE -= -g
QMAKE_CXXFLAGS += $$(QT_CXXFLAGS)

win32:DEFINES += _CRT_SECURE_NO_WARNINGS SYNTROGUI_LIB

INCLUDEPATH += GeneratedFiles \
    GeneratedFiles/Release \
    ../SyntroLib

win32* {
    LIBS += -L../SyntroLib/Release -lSyntroLib
}
else {
    LIBS += -L../SyntroLib -lSyntroLib
}

MOC_DIR += GeneratedFiles/release

OBJECTS_DIR += release

UI_DIR += GeneratedFiles

RCC_DIR += GeneratedFiles

include(SyntroGUI.pri)
