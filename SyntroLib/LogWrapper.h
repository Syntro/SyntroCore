//
//  Copyright (c) 2012 Pansenti, LLC.
//	
//  This file is part of SyntroLib
//
//  SyntroLib is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  SyntroLib is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with SyntroLib.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LOGWRAPPER_H
#define LOGWRAPPER_H

#include <qsettings.h>

#include "syntrolib_global.h"
#include "Logger.h"


SYNTROLIB_EXPORT bool logCreate(QSettings *settings);
SYNTROLIB_EXPORT void logDestroy();
SYNTROLIB_EXPORT void logDebug(QString str);
SYNTROLIB_EXPORT void logInfo(QString str);
SYNTROLIB_EXPORT void logWarn(QString str);
SYNTROLIB_EXPORT void logError(QString str);
SYNTROLIB_EXPORT QQueue<LogMessage>* activeStreamQueue();


#endif // LOGWRAPPER_H
