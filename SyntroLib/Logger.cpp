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

#include <QtGlobal>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <qdebug.h>

#include "SyntroDefs.h"
#include "SyntroUtils.h"
#include "Logger.h"

#define MAX_STREAM_QUEUE_MESSAGES 64

Logger::Logger(QString appType, QString appName, int level, bool diskLog, bool netLog, int logKeep)
{
    m_thread = NULL;
    m_activeDiskQ = 0;
	m_activeStreamQ = 0;
    m_logInterval = LOG_FLUSH_INTERVAL_SECONDS;
	m_logKeep = logKeep;

	m_appName = appName;
	m_diskLog = diskLog;
	m_netLog = netLog;
	
	if (m_appName.length() < 1)
		m_appName = "Unknown";

	if (appType.length() > 0)
		m_logName = appType + "-" + m_appName + ".log";
	else
		m_logName = m_appName + ".log";

	m_logName.replace(' ', '_');

 	if (m_diskLog) {
		if (diskOpen()) {
			m_thread = new LogThread();
			m_thread->m_log = this;
			m_thread->start();
		}
		else {
			m_diskLog = false;
		}
	}

	if (level >= SYNTRO_LOG_LEVEL_INFO) {
        logWrite("INFO", QString("Log start: %1").arg(m_logName.left(m_logName.length() - 4)));

		if (level == SYNTRO_LOG_LEVEL_DEBUG)
			logWrite("DEBUG", QString("Qt  runtime %1  build %2").arg(qVersion()).arg(QT_VERSION_STR));
	}
}

Logger::~Logger()
{
    if (m_thread) {
		m_thread->m_stop = true;
        m_thread->wait();
        delete m_thread;
        m_thread = NULL;
    }

    if (m_file.isOpen())
		m_file.close();
}

void Logger::logWrite(QString level, QString str)
{
	LogMessage m(level, str);

	if (m_diskLog) {
		if (m_flushMutex.tryLock()) {
			m_diskQ[m_activeDiskQ].enqueue(m);
			m_flushMutex.unlock();
		}
	}

	if (m_netLog) {
		if (m_streamMutex.tryLock()) {
			// throw away older messages
			// TODO: be smarter about this, maybe xxx repeated n times...
			if (m_streamQ[m_activeStreamQ].count() > MAX_STREAM_QUEUE_MESSAGES)
				m_streamQ[m_activeStreamQ].dequeue();

			m_streamQ[m_activeStreamQ].enqueue(m);
			m_streamMutex.unlock();
		}
	}
}

bool Logger::diskOpen()
{
    QString f;

	if (m_file.isOpen())
        return true;

	if (m_logKeep > 0) {
		if (!rotateLogs(m_logName))
			return false;
	}

	m_file.setFileName(m_logName);

	if (!m_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return false;

	m_stream.setDevice(&m_file);

    return true;
}

// Remove logName.<m_logKeep>
// Rename logName.<m_logKeep - 1> to logName.<m_logKeep>
// etc...down to
// Rename logName to logName.1
// So that we can then open logName for use.
bool Logger::rotateLogs(QString logName)
{
	QFile file;
	QFileInfo info;
	QString src, dst;

	dst = logName + '.' + QString::number(m_logKeep);

	info.setFile(dst);

	if (info.exists()) {
		if (!QFile::remove(dst)) {
			qDebug() << "QFile.remove(" << dst << ") failed";
			return false;
		}
	}

	for (int i = m_logKeep; i > 1; i--) {
		src = logName + '.' + QString::number(i - 1);

		info.setFile(src);

		if (info.exists()) {
			dst = logName + '.' + QString::number(i);

			if (!QFile::rename(src, dst)) {
				qDebug() << "QFile.rename(" << src << ", " << dst << ") failed";
				return false;
			}
		}
	}

	info.setFile(logName);

	if (info.exists()) {
		dst = logName + ".1";

		if (!QFile::rename(logName, dst)) {
			qDebug() << "QFile.rename(" << logName << ", " << dst << ") failed";
			return false;
		}
	}

	return true;
}

// Switch the active queue and write out the non-active queue.
void Logger::diskFlush()
{
    int write_queue = m_activeDiskQ;

    if (!m_file.isOpen()) {
        if (!diskOpen())
            return;
    }

    m_flushMutex.lock();
    m_activeDiskQ = (m_activeDiskQ == 0 ? 1 : 0);
    m_flushMutex.unlock();

 	while (!m_diskQ[write_queue].empty()) {
 		LogMessage next = m_diskQ[write_queue].dequeue();

#ifdef WIN32
		m_stream << next.m_level << " " << next.m_timeStamp << " " << next.m_msg << '\r' << endl;
#else
		m_stream << next.m_level << " " << next.m_timeStamp << " " << next.m_msg << endl;
#endif
    }
}

void LogThread::run()
{
    while (!m_stop) {
		sleep(m_log->m_logInterval);
		m_log->diskFlush();
    }

	m_log->diskFlush();

    exit(0);
}

QQueue<LogMessage>* Logger::streamQueue()
{
	m_streamMutex.lock();

	int n = m_activeStreamQ;

	m_activeStreamQ = m_activeStreamQ ? 0 : 1;

	m_streamQ[m_activeStreamQ].clear();

	m_streamMutex.unlock();

	return &m_streamQ[n];
}

LogMessage::LogMessage(QString level, QString &msg)
{
	m_level = level;
	QDateTime dt = QDateTime::currentDateTime();
	m_timeStamp = QString("%1.%2").arg(dt.toString(Qt::ISODate)).arg(dt.time().msec(), 3, 10, QChar('0'));
	m_msg = msg;
}

LogMessage::LogMessage(const LogMessage &rhs)
{
	m_level = rhs.m_level;
	m_msg = rhs.m_msg;
	m_timeStamp = rhs.m_timeStamp;
}

LogMessage& LogMessage::operator=(const LogMessage &rhs)
{
	if (this != &rhs) {
		m_level = rhs.m_level;
		m_msg = rhs.m_msg;
		m_timeStamp = rhs.m_timeStamp;
	}

	return *this;
}
