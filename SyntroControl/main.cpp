//
//  Copyright (c) 2012 Pansenti, LLC.
//	
//  This file is part of Syntro
//
//  Syntro is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Syntro is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Syntro.  If not, see <http://www.gnu.org/licenses/>.
//

#include "SyntroControl.h"
#include <QApplication>

#include "SyntroUtils.h"
#include "ControlConsole.h"


int runGuiApp(int argc, char *argv[]);
int runConsoleApp(int argc, char *argv[]);
QSettings *loadSettings(QStringList arglist);


int main(int argc, char *argv[])
{
	if (checkConsoleModeFlag(argc, argv))
		return runConsoleApp(argc, argv);
	else
		return runGuiApp(argc, argv);
}

// look but do not modify argv

int runGuiApp(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QSettings *settings = loadSettings(a.arguments());

	SyntroControl *w = new SyntroControl(settings);

	w->show();

	return a.exec();
}

int runConsoleApp(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QSettings *settings = loadSettings(a.arguments());

	ControlConsole cc(settings, &a);

	return a.exec();
}

QSettings *loadSettings(QStringList arglist)
{
	QSettings *settings = loadStandardSettings(PRODUCT_TYPE, arglist);

	// app-specific part

	settings->setValue(SYNTRO_PARAMS_COMPTYPE, PRODUCT_TYPE);

	return settings;
}

