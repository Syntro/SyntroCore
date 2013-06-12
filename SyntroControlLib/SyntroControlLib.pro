# This file is part of Syntro
#
# Copyright (c) 2012 Pansenti, LLC. All rights reserved.
#

TEMPLATE = lib
TARGET = SyntroControlLib

win32* {
	DESTDIR = Release 
}

include(../version.pri)


# common mac or linux
unix {
        target.path = /usr/lib
	headerfiles.path = /usr/include/syntro/SyntroControlLib
	headerfiles.files += *.h

	INSTALLS += headerfiles target
}

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += shared

# No debug info in release builds
unix:QMAKE_CXXFLAGS_RELEASE -= -g
QMAKE_CXXFLAGS += $$(QT_CXXFLAGS)

DEFINES += QT_NETWORK_LIB

win32:DEFINES += _CRT_SECURE_NO_WARNINGS SYNTROCONTROLLIB_LIB

INCLUDEPATH += GeneratedFiles \
    GeneratedFiles/release \
    ../SyntroLib

win32* {
    LIBS += -L../SyntroLib/Release -lSyntroLib
} else {
    LIBS += -L../SyntroLib -lSyntroLib
}

MOC_DIR += GeneratedFiles/release

OBJECTS_DIR += release

UI_DIR += GeneratedFiles

RCC_DIR += GeneratedFiles

include(SyntroControlLib.pri)
