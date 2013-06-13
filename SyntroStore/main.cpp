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

#include "SyntroStore.h"
#include <QApplication>

#include <QtDebug>

#include "SyntroUtils.h"
#include "SyntroStoreConsole.h"
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

	SyntroStore *w = new SyntroStore(settings);

	w->show();

	return a.exec();
}

int runConsoleApp(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QSettings *settings = loadSettings(a.arguments());

	SyntroStoreConsole sc(settings, &a);

	return a.exec();
}

QSettings *loadSettings(QStringList arglist)
{
	QSettings *settings = SyntroUtils::loadStandardSettings(PRODUCT_TYPE, arglist);

	// app-specific part

	settings->setValue(SYNTRO_PARAMS_COMPTYPE, PRODUCT_TYPE);

	// Max age of files in days afterwhich they will be deleted
	// A value of 0 turns off the delete behavior
	if (!settings->contains(SYNTROSTORE_MAXAGE))
		settings->setValue(SYNTROSTORE_MAXAGE, 0);		

	// The SyntroStore component can save any type of stream.
	// Here you can list the Syntro streams by name that it should look for.
	int	nSize = settings->beginReadArray(SYNTROSTORE_PARAMS_STREAM_SOURCES);
	settings->endArray();

	if (nSize == 0) {
		settings->beginWriteArray(SYNTROSTORE_PARAMS_STREAM_SOURCES);
	
		for (int index = 0; index < SYNTROSTORE_MAX_STREAMS; index++) {
			settings->setArrayIndex(index);
			if (index == 0) {
				settings->setValue(SYNTROSTORE_PARAMS_INUSE, SYNTRO_PARAMS_TRUE);
				settings->setValue(SYNTROSTORE_PARAMS_STREAM_SOURCE, "source/video");
			} else {
				settings->setValue(SYNTROSTORE_PARAMS_INUSE, SYNTRO_PARAMS_FALSE);
				settings->setValue(SYNTROSTORE_PARAMS_STREAM_SOURCE, "");
			}
			settings->setValue(SYNTROSTORE_PARAMS_FORMAT, SYNTRO_RECORD_STORE_FORMAT_SRF);
			settings->setValue(SYNTROSTORE_PARAMS_PATH, "./");
			settings->setValue(SYNTROSTORE_PARAMS_CREATE_SUBFOLDER, SYNTRO_PARAMS_TRUE);
			settings->setValue(SYNTROSTORE_PARAMS_ROTATION_POLICY, 	SYNTROSTORE_PARAMS_ROTATION_POLICY_TIME);
			settings->setValue(SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS, 	SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS_HOURS);
			settings->setValue(SYNTROSTORE_PARAMS_ROTATION_TIME, "2");
			settings->setValue(SYNTROSTORE_PARAMS_ROTATION_SIZE, "256");
		}
		settings->endArray();
	}

	return settings;
}
