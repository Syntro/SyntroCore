# This file is part of Syntro
#
# Copyright (c) 2013 Pansenti, LLC. All rights reserved.
#

cache()

TEMPLATE = subdirs

SUBDIRS = SyntroLib \
        SyntroGUI \
        SyntroControlLib \
        SyntroControl \
        SyntroDB \
        SyntroLog

SyntroGUI.depends = SyntroLib
SyntroControlLib.depends = SyntroLib
SyntroControl.depends = SyntroLib SyntroGUI SyntroControlLib
SyntroLog.depends = SyntroLib SyntroGUI
SyntroDB.depends = SyntroLib SyntroGUI

