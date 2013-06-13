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

#include "SyntroLog.h"
#include <QApplication>

#include "SyntroUtils.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QSettings *settings = SyntroUtils::loadStandardSettings(PRODUCT_TYPE, a.arguments());

	settings->setValue(SYNTRO_PARAMS_COMPTYPE, PRODUCT_TYPE);

	SyntroLog *w = new SyntroLog(settings);

	w->show();

	return a.exec();
}
