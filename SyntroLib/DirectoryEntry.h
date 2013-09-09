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

#ifndef DIRECTORYENTRY_H
#define DIRECTORYENTRY_H

#include "syntrolib_global.h"
#include <qstring.h>

class SYNTROLIB_EXPORT DirectoryEntry
{
public:
	DirectoryEntry(QString dirLine = "");
	
	void setLine(QString dirLine);

	bool isValid();

	QString uid();
	QString appName();
	QString componentType();
	QString multicastService();
	QString e2eService();

private:
	void parseLine();
	QString element(QString name);

	QString m_raw;
	QString m_uid;
	QString m_name;
	QString m_type;
	QString m_multicastService;
	QString m_e2eService;
};

#endif // DIRECTORYENTRY_H
