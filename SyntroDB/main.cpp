//
//  Copyright (c) 2013 Pansenti, LLC.
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

#include "SyntroDB.h"
#include <QApplication>
#include <qdir.h>

#include <QtDebug>

#include "SyntroUtils.h"
#include "SyntroDBConsole.h"
#include "SyntroRecord.h"

int runGuiApp(int argc, char *argv[]);
int runConsoleApp(int argc, char *argv[]);
void loadSettings(QStringList arglist);


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

	loadSettings(a.arguments());

	SyntroDB *w = new SyntroDB();

	w->show();

	return a.exec();
}

int runConsoleApp(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	loadSettings(a.arguments());

	SyntroDBConsole sc(&a);

	return a.exec();
}

void loadSettings(QStringList arglist)
{
	SyntroUtils::loadStandardSettings(APPTYPE_DB, arglist);

	// app-specific part

	QSettings *settings = SyntroUtils::getSettings();

	if (!settings->contains(SYNTRODB_PARAMS_ROOT_DIRECTORY))
		settings->setValue(SYNTRODB_PARAMS_ROOT_DIRECTORY, QDir::homePath() + "/Syntro/");

	delete settings;
}
