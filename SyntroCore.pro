# This file is part of Syntro
#
# Copyright (c) 2012 Pansenti, LLC. All rights reserved.
#

TEMPLATE = subdirs

SUBDIRS = SyntroLib \
        SyntroGUI \
        SyntroControlLib \
        SyntroCFS \
        SyntroControl \
        SyntroExec \
        SyntroLog \
        SyntroStore

SyntroGUI.depends = SyntroLib
SyntroControlLib.depends = SyntroLib
SyntroCFS.depends = SyntroLib SyntroGUI
SyntroControl.depends = SyntroLib SyntroGUI SyntroControlLib
SyntroExec.depends = SyntroLib SyntroGUI
SyntroLog.depends = SyntroLib SyntroGUI
SyntroStore.depends = SyntroLib SyntroGUI

