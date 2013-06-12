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

#include "LogClient.h"
#include "SyntroLib.h"

#define	LOGCLIENT_BACKGROUND_INTERVAL (SYNTRO_CLOCKS_PER_SEC / 10)

// in LOGCLIENT_BACKGROUND_INTERVAL units, so 5 seconds
#define LOGCLIENT_DIR_REFRESH_INTERVAL 50

LogClient::LogClient(QObject *parent, QSettings *settings)
	: Endpoint(parent, settings, LOGCLIENT_BACKGROUND_INTERVAL)
{
	// force a dir refresh as soon as we connect
	m_dirRefreshCounter = LOGCLIENT_DIR_REFRESH_INTERVAL;
	m_waitingOnDirRefresh = false;
}

void LogClient::appClientInit()
{
}

void LogClient::appClientConnected()
{
	logLocal("SyntroLink established");
}

void LogClient::appClientClosed()
{
	logLocal("SyntroLink closed");
}

void LogClient::appClientReceiveMulticast(int servicePort, SYNTRO_EHEAD *multiCast, int len)
{
	SYNTRO_RECORD_HEADER *recordHead = reinterpret_cast<SYNTRO_RECORD_HEADER *>(multiCast + 1);

	if (convertUC2ToInt(recordHead->type) == SYNTRO_RECORD_TYPE_LOG) {
		int dataLength = len - convertUC2ToInt(recordHead->headerLength);

		if (dataLength > 0) {
			QByteArray bulkMsg(reinterpret_cast<const char *>(recordHead + 1), dataLength);
			emit newLogMsg(bulkMsg);
		}
	}
		
	clientSendMulticastAck(servicePort);

	free(multiCast);
}

void LogClient::appClientBackground()
{
	if (clientIsConnected()) {
		m_dirRefreshCounter++;

		if (m_dirRefreshCounter > LOGCLIENT_DIR_REFRESH_INTERVAL) {
			if (m_waitingOnDirRefresh) {
				logLocal(QString("Failed to get directory response after %1 seconds").arg((LOGCLIENT_BACKGROUND_INTERVAL * LOGCLIENT_DIR_REFRESH_INTERVAL) / 1000));
				m_waitingOnDirRefresh = false;
			}
			else {
				requestDirectory();
				m_waitingOnDirRefresh = true;
				m_dirRefreshCounter = 0;
			}
		}
	}
}

void LogClient::appClientReceiveDirectory(SYNTRO_DIRECTORY_RESPONSE *directory, int length)
{
	int oldCount = m_sources.count();

	// don't want the header or the trailing NULL byte
	QByteArray rawDir(reinterpret_cast<char *>(directory + 1), length - (1 + sizeof(SYNTRO_DIRECTORY_RESPONSE)));

	// it's wasteful for SyntroControl to be sending this, but until that gets fixed ...
	QByteArray stripDir = rawDir.replace("<NSV></NSV>", "");

	// done with this
	free(directory);

	// mark all inactive
	for (int i = 0; i < m_sources.count(); i++)
		m_sources[i].m_active = false;

	QList<QByteArray> dirList = stripDir.split(0);
	
	for (int i = 0; i < dirList.count(); i++)
		handleDirEntry(QString(dirList[i]));

	// any entries not active now should be removed
	for (int i = 0; i < m_sources.count(); i++) {
		if (!m_sources[i].m_active) {
			clientRemoveService(m_sources[i].m_port);
			logLocal(QString("Removed client %1").arg(m_sources[i].m_name));
			m_sources.removeAt(i);
		}
	}

	if (m_sources.count() != oldCount)
		emit activeClientUpdate(m_sources.count());

	m_waitingOnDirRefresh = false;
	m_dirRefreshCounter = 0;
}

void LogClient::handleDirEntry(QString dirEntry)
{
	int start = dirEntry.indexOf("<MSV>");

	while (start >= 0) {
		int end = dirEntry.indexOf("</MSV>");

		QString entry = dirEntry.mid(start + 5, end - (start + 5));

		if (entry.endsWith(SYNTRO_LOG_SERVICE_TAG)) {
			int i = findEntry(entry);
		
			if (i >= 0) {
				m_sources[i].m_active = true;
			}
			else {
				int port = clientAddService(entry, SERVICETYPE_MULTICAST, false, true);

				if (port >= 0) {
					logLocal(QString("New client %1").arg(entry));
					m_sources.append(LogStreamEntry(entry, true, port));
				}
			}
		}

		dirEntry = dirEntry.right(dirEntry.length() - (end + 6));
		start = dirEntry.indexOf("<MSV>");
	}
}

int LogClient::findEntry(QString name)
{
	for (int i = 0; i < m_sources.count(); i++) {
		if (m_sources[i].m_name == name)
			return i;
	}

	return -1;
}

void LogClient::logLocal(QString s)
{
	QByteArray msg;

	QDateTime dt = QDateTime::currentDateTime();
	QString timestamp = QString("%1.%2").arg(dt.toString(Qt::ISODate)).arg(dt.time().msec(), 3, 10, QChar('0'));

	msg.append("LOCAL|");
	msg.append(timestamp);
	msg.append("|local|local|");
	msg.append(s);

	emit newLogMsg(msg);
}

LogStreamEntry::LogStreamEntry(QString name, bool active, int port)
{
	m_name = name;
	m_active = active;
	m_port = port;
}