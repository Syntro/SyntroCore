//
//  Copyright (c) 2012, 2013 Pansenti, LLC.
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

#define logDebug(msg)	_logDebug(m_logTag + "--> " + msg);
#define logInfo(msg)	_logInfo(m_logTag + "--> " + msg);
#define logWarn(msg)	_logWarn(m_logTag + "--> " + msg);
#define logError(msg)	_logError(m_logTag + "--> " + msg);

SYNTROLIB_EXPORT bool logCreate();
SYNTROLIB_EXPORT void logDestroy();
SYNTROLIB_EXPORT void _logDebug(QString str);
SYNTROLIB_EXPORT void _logInfo(QString str);
SYNTROLIB_EXPORT void _logWarn(QString str);
SYNTROLIB_EXPORT void _logError(QString str);

#endif // LOGWRAPPER_H
