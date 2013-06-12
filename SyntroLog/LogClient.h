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

#ifndef LOGCLIENT_H
#define LOGCLIENT_H

#include "SyntroLib.h"

class LogStreamEntry
{
public:
	LogStreamEntry(QString name, bool active, int port);
	QString m_name;
	bool m_active;
	int m_port;
};

class LogClient : public Endpoint
{
	Q_OBJECT

public:
	LogClient(QObject *parent, QSettings *settings);

signals:
	void newLogMsg(QByteArray bulkMsg);
	void activeClientUpdate(int count);

protected:
	void appClientInit();
	void appClientReceiveMulticast(int servicePort, SYNTRO_EHEAD *multiCast, int len);
	void appClientBackground();
	void appClientReceiveDirectory(SYNTRO_DIRECTORY_RESPONSE *directory, int length);
	void appClientConnected();
	void appClientClosed();

private:
	void handleDirEntry(QString dirEntry);
	int findEntry(QString name);
	void logLocal(QString msg);

	bool m_waitingOnDirRefresh;
	int m_dirRefreshCounter;
	
	QList<LogStreamEntry> m_sources;
};

#endif // LOGCLIENT_H

