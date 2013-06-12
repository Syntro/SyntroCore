# This file is part of Syntro
#
# Copyright (c) 2012 Pansenti, LLC. All rights reserved.
#

TEMPLATE = lib
TARGET = SyntroLib

win32* {
	DESTDIR = Release 
}

include(../version.pri)

# common mac or linux
unix {
        target.path = /usr/lib

	headerfiles.path = /usr/include/syntro
	headerfiles.files += *.h

	avheaderfiles.path = /usr/include/syntro/SyntroAV
	avheaderfiles.files += SyntroAV/*.h

	robotheaderfiles.path = /usr/include/syntro/SyntroRobot
	robotheaderfiles.files += SyntroRobot/*.h

        pkgconfigfiles.path = /usr/lib/pkgconfig
        pkgconfigfiles.files = syntro.pc

	INSTALLS += headerfiles \
		avheaderfiles \
		robotheaderfiles \
                pkgconfigfiles \
		target
}

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += shared

# No debug info in release builds
unix:QMAKE_CXXFLAGS_RELEASE -= -g
QMAKE_CXXFLAGS += $$(QT_CXXFLAGS)

DEFINES += QT_NETWORK_LIB

win32:DEFINES += _CRT_SECURE_NO_WARNINGS SYNTROLIB_LIB

INCLUDEPATH += GeneratedFiles \
    GeneratedFiles/release 

MOC_DIR += GeneratedFiles/release

OBJECTS_DIR += release

UI_DIR += GeneratedFiles

RCC_DIR += GeneratedFiles

include(SyntroLib.pri)
