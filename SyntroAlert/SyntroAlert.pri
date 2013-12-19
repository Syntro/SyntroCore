# This file is part of Syntro
#
# Copyright (c) 2013 Pansenti, LLC. All rights reserved.
#

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += AlertClient.h \
    MessageTypeDlg.h \
	SyntroAlert.h \
    ViewFieldsDlg.h

SOURCES += AlertClient.cpp \
    main.cpp \
    MessageTypeDlg.cpp \
    SyntroAlert.cpp \
    ViewFieldsDlg.cpp

FORMS += syntroalert.ui
