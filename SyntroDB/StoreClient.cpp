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

#include "SyntroDB.h"
#include "SyntroDBConsole.h"
#include "StoreClient.h"
#include "SyntroLib.h"
#include "StoreDiskManager.h"

StoreClient::StoreClient(QObject *parent)
	: Endpoint(SYNTROSTORE_BGND_INTERVAL, COMPTYPE_STORE),  m_parent(parent)
{
	for (int i = 0; i < SYNTRODB_MAX_STREAMS; i++) {
		m_sources[i] = NULL;
		m_diskManagers[i] = NULL;
	}
}

void StoreClient::appClientInit()
{

}

void StoreClient::appClientExit()
{
	for (int i = 0; i < SYNTRODB_MAX_STREAMS; i++) {
		deleteStreamSource(i);
	}	
}

void StoreClient::appClientReceiveMulticast(int servicePort, SYNTRO_EHEAD *message, int len)
{
	int sourceIndex;

	sourceIndex = clientGetServiceData(servicePort);

	if ((sourceIndex >= SYNTRODB_MAX_STREAMS) || (sourceIndex < 0)) {
		logWarn(QString("Multicast received to out of range port %1").arg(servicePort));
		free(message);
		return;
	}
	if (m_sources[sourceIndex] != NULL) {					// still active
		m_sources[sourceIndex]->queueBlock(QByteArray(reinterpret_cast<char *>(message + 1), len));
		clientSendMulticastAck(servicePort);
	}
	free(message);
}


void StoreClient::refreshStreamSource(int index)
{
	QMutexLocker locker(&m_lock);

	QSettings *settings = SyntroUtils::getSettings();

	QString rootDirectory = settings->value(SYNTRODB_PARAMS_ROOT_DIRECTORY).toString();
	int count = settings->beginReadArray(SYNTRODB_PARAMS_STREAM_SOURCES);

	if ((index < 0) || (index >= count)) {					// no more entries in settings
		settings->endArray();
		return;
	} else {
		settings->setArrayIndex(index);
		if (m_diskManagers[index] != NULL) {					// must get rid of old one
			deleteStreamSource(index);
			m_diskManagers[index] = NULL;
		}

		if (settings->value(SYNTRODB_PARAMS_INUSE).toString() == SYNTRO_PARAMS_FALSE) {
			settings->endArray();
			return;											// just needed to clear the entry
		}
		m_sources[index] = new StoreStream(m_logTag);
		m_diskManagers[index] = new StoreDiskManager(m_sources[index]);
		m_sources[index]->load(settings, rootDirectory);
		m_diskManagers[index]->resumeThread();

		if (m_sources[index]->folderWritable()) {
			m_sources[index]->port = clientAddService(m_sources[index]->streamName(), SERVICETYPE_MULTICAST, false);
			clientSetServiceData(m_sources[index]->port, index);	// record my index in service entry
		} else {
			logError(QString("Folder is not writable: %1").arg(m_sources[index]->pathOnly()));
		}
	}
	settings->endArray();

	delete settings;
}

void StoreClient::deleteStreamSource(int index)
{
	if ((index < 0) || (index >= SYNTRODB_MAX_STREAMS)) {
		logWarn(QString("Got deleteStreamSource with out of range index %1").arg(index));
		return;
	}

	if (m_diskManagers[index] == NULL)
		return;												// not currently active

	clientRemoveService(m_sources[index]->port);
	m_diskManagers[index]->exitThread();
	m_diskManagers[index]->thread()->wait(5);
	delete m_sources[index];
	m_sources[index] = NULL;
}
