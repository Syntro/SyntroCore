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

#include <QApplication>

#include "SyntroExec.h"
#include "ExecConsole.h"
#include "SyntroUtils.h"


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

	SyntroExec *w = new SyntroExec(settings);

	w->show();

	return a.exec();
}

int runConsoleApp(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QSettings *settings = loadSettings(a.arguments());

	ExecConsole cc(settings, &a);

	return a.exec();
	return 1;
}

QSettings *loadSettings(QStringList arglist)
{
	QSettings *settings = loadStandardSettings(PRODUCT_TYPE, arglist);

	// app-specific part

	settings->setValue(SYNTRO_PARAMS_COMPTYPE, COMPTYPE_EXEC);

	int	size = settings->beginReadArray(SYNTROEXEC_PARAMS_COMPONENTS);
	settings->endArray();

	if (size == 0) {
		settings->beginWriteArray(SYNTROEXEC_PARAMS_COMPONENTS);
		settings->setArrayIndex(0);

	// by default, just configure a SyntroControl with default runtime arguments

		settings->setValue(SYNTROEXEC_PARAMS_INUSE, SYNTRO_PARAMS_TRUE);
		settings->setValue(SYNTROEXEC_PARAMS_APP_NAME, COMPTYPE_CONTROL);
		settings->setValue(SYNTROEXEC_PARAMS_EXECUTABLE_DIRECTORY, "");
		settings->setValue(SYNTROEXEC_PARAMS_WORKING_DIRECTORY, "");
		settings->setValue(SYNTROEXEC_PARAMS_ADAPTOR, "");
		settings->setValue(SYNTROEXEC_PARAMS_INI_PATH, "");
		settings->setValue(SYNTROEXEC_PARAMS_CONSOLE_MODE, SYNTRO_PARAMS_FALSE);
		settings->setValue(SYNTROEXEC_PARAMS_MONITORED, SYNTRO_PARAMS_TRUE);

	// and something for SyntroExec

		settings->setArrayIndex(1);
		settings->setValue(SYNTROEXEC_PARAMS_APP_NAME, COMPTYPE_EXEC);

		settings->endArray();
	}
	return settings;
}

