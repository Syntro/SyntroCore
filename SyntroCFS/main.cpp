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

#include "SyntroCFS.h"
#include <QApplication>

#include <QtDebug>

#include "SyntroUtils.h"
#include "SyntroCFSConsole.h"
#include "SyntroRecord.h"

int runGuiApp(int argc, char *argv[]);
int runConsoleApp(int argc, char *argv[]);
QSettings *loadSettings(QStringList arglist);


int main(int argc, char *argv[])
{
	if (SyntroUtils::checkConsoleModeFlag(argc, argv))
		return runConsoleApp(argc, argv);
	else
		return runGuiApp(argc, argv);
}

// look but do not modify argv

int runGuiApp(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QSettings *settings = loadSettings(a.arguments());

	SyntroCFS *w = new SyntroCFS(settings);

	w->show();

	return a.exec();
}

int runConsoleApp(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QSettings *settings = loadSettings(a.arguments());

	SyntroCFSConsole sc(settings, &a);

	return a.exec();
}

QSettings *loadSettings(QStringList arglist)
{
	QSettings *settings = SyntroUtils::loadStandardSettings(PRODUCT_TYPE, arglist);

	// app-specific part

	settings->setValue(SYNTRO_PARAMS_COMPTYPE, PRODUCT_TYPE);

	// Where the files will be stored
	if (!settings->contains(SYNTRO_CFS_STORE_PATH))
		settings->setValue(SYNTRO_CFS_STORE_PATH, "./");

	return settings;
}
