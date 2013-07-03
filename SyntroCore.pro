# This file is part of Syntro
#
# Copyright (c) 2012 Pansenti, LLC. All rights reserved.
#

TEMPLATE = subdirs

SUBDIRS = SyntroLib \
        SyntroGUI \
        SyntroControlLib \
        SyntroControl \
        SyntroExec \
	   SyntroDB \
        SyntroLog
SyntroGUI.depends = SyntroLib
SyntroControlLib.depends = SyntroLib
SyntroControl.depends = SyntroLib SyntroGUI SyntroControlLib
SyntroExec.depends = SyntroLib SyntroGUI
SyntroLog.depends = SyntroLib SyntroGUI
SyntroDB.depends = SyntroLib SyntroGUI

