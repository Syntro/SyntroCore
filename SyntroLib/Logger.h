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

#ifndef LOGGER_H
#define LOGGER_H

#include "syntrolib_global.h"
#include <qthread.h>
#include <qmutex.h>
#include <qfile.h>
#include <qwaitcondition.h>
#include <qtextstream.h>
#include <qqueue.h>


#define LOG_FLUSH_INTERVAL_SECONDS 1

class SYNTROLIB_EXPORT LogMessage
{
public:
	LogMessage(QString level, QString &msg);
	LogMessage(const LogMessage &rhs);

	LogMessage& operator=(const LogMessage &rhs);

	QString m_level;
	QString m_msg;
    QString m_timeStamp;
};

class Logger;

class LogThread : public QThread
{
public:
    LogThread() : m_log(NULL), m_stop(false) {}
    void run();

    Logger *m_log;
    bool m_stop;
};

class Logger
{
friend class LogThread;

public:
	Logger(QString appType, QString appName, int level, bool diskLog, bool netLog, int logKeep);

	~Logger();

	void logWrite(QString level, QString str);
	QQueue<LogMessage>* streamQueue();

private:
    bool diskOpen();
    void diskFlush();
	bool rotateLogs(QString logName);

	QString m_logName;
	QString m_appName;
	bool m_diskLog;
	bool m_netLog;
	int m_logKeep;
	QFile m_file;
	QTextStream m_stream;
	LogThread *m_thread;
	QWaitCondition m_stopCondition;
	QMutex m_stopMutex;
    QMutex m_flushMutex;
	QMutex m_streamMutex;
	QQueue<LogMessage> m_diskQ[2];
	QQueue<LogMessage> m_streamQ[2];
    int m_logInterval;
    int m_activeDiskQ;
	int m_activeStreamQ;
};


#endif // LOGGER_H
