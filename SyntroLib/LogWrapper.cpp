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

#include <qsettings.h>
#include "SyntroDefs.h"
#include "SyntroUtils.h"
#include "Logger.h"

#include "LogWrapper.h"

Logger *logSingleton = NULL;
int logLevel = SYNTRO_LOG_LEVEL_INFO;


bool logCreate()
{
	if (logSingleton)
		return false;

	QSettings *settings = SyntroUtils::getSettings();

	QString appName = settings->value(SYNTRO_PARAMS_APPNAME).toString();

	settings->beginGroup(SYNTRO_PARAMS_LOG_GROUP);

	bool diskLog = settings->value(SYNTRO_PARAMS_DISK_LOG, true).toBool();
	bool netLog = settings->value(SYNTRO_PARAMS_NET_LOG, true).toBool();

	int logKeep = settings->value(SYNTRO_PARAMS_LOG_KEEP, -1).toInt();

	if (logKeep < 0)
		logKeep = SYNTRO_DEFAULT_LOG_KEEP;
	else if (logKeep > 20)
		logKeep = 20;

	QString level = settings->value(SYNTRO_PARAMS_LOGLEVEL, QString("info")).toString().toLower();

	if (level == "error")
		logLevel = SYNTRO_LOG_LEVEL_ERROR;
	else if (level == "warn")
		logLevel = SYNTRO_LOG_LEVEL_WARN;
	else if (level == "info")
		logLevel = SYNTRO_LOG_LEVEL_INFO;
	else if (level == "debug")
		logLevel = SYNTRO_LOG_LEVEL_DEBUG;
	else {
		logLevel = SYNTRO_LOG_LEVEL_INFO;
		level = "info";
	}

	// write out what we are using
	settings->setValue(SYNTRO_PARAMS_LOGLEVEL, level);
	settings->setValue(SYNTRO_PARAMS_DISK_LOG, diskLog);
	settings->setValue(SYNTRO_PARAMS_NET_LOG, netLog);
	settings->setValue(SYNTRO_PARAMS_LOG_KEEP, logKeep);

	settings->endGroup();

	logSingleton = new Logger(appName, logLevel, diskLog, netLog, logKeep);
	logSingleton->setHeartbeatTimers(
			settings->value(SYNTRO_PARAMS_LOG_HBINTERVAL, SYNTRO_LOG_HEARTBEAT_INTERVAL).toInt(),
			settings->value(SYNTRO_PARAMS_LOG_HBTIMEOUT, SYNTRO_LOG_HEARTBEAT_TIMEOUT).toInt());

	logSingleton->resumeThread();

	delete settings;

	return true;
}

void logDestroy()
{
	if (logSingleton) {
		logSingleton->exitThread();
		logSingleton = NULL;
	}
}

void logAny(QString level, QString str)
{
	if (logSingleton)
		logSingleton->logWrite(level, str);
}

void _logDebug(QString str)
{
	if (logLevel < SYNTRO_LOG_LEVEL_DEBUG)
		return;

	logAny("DEBUG", str);

	qDebug() << qPrintable(str);
}

void _logInfo(QString str)
{
	if (logLevel < SYNTRO_LOG_LEVEL_INFO)
		return;

	logAny("INFO", str);

	if (logLevel >= SYNTRO_LOG_LEVEL_DEBUG)
		qDebug() << qPrintable(str);
}

void _logWarn(QString str)
{
	if (logLevel < SYNTRO_LOG_LEVEL_WARN)
		return;

	logAny("WARN", str);

	qWarning() << qPrintable(str);
}

void _logError(QString str)
{
	if (logLevel < SYNTRO_LOG_LEVEL_ERROR)
		return;

	logAny("ERROR", str);

	qCritical() << qPrintable(str);
}

QQueue<LogMessage>* activeStreamQueue()
{
	if (!logSingleton)
		return NULL;

	return logSingleton->streamQueue();
}
