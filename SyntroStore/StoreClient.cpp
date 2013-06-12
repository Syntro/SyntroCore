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

#include "SyntroStore.h"
#include "SyntroStoreConsole.h"
#include "StoreClient.h"
#include "SyntroLib.h"
#include "StoreDiskManager.h"

StoreClient::StoreClient(QObject *parent, QSettings *settings)
	: Endpoint(parent, settings, SYNTROSTORE_BGND_INTERVAL),  m_parent(parent)
{
	for (int i = 0; i < SYNTROSTORE_MAX_STREAMS; i++) {
		m_sources[i] = NULL;
		m_diskManagers[i] = NULL;
	}
}

void StoreClient::appClientInit()
{

}

void StoreClient::appClientExit()
{
	for (int i = 0; i < SYNTROSTORE_MAX_STREAMS; i++) {
		deleteStreamSource(i);
	}	
}

void StoreClient::appClientReceiveMulticast(int servicePort, SYNTRO_EHEAD *message, int len)
{
	int sourceIndex;

	sourceIndex = clientGetServiceData(servicePort);

	if ((sourceIndex >= SYNTROSTORE_MAX_STREAMS) || (sourceIndex < 0)) {
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

	int count = m_settings->beginReadArray(SYNTROSTORE_PARAMS_STREAM_SOURCES);

	if ((index < 0) || (index >= count)) {					// no more entries in settings
		m_settings->endArray();
		return;
	} else {
		m_settings->setArrayIndex(index);
		if (m_diskManagers[index] != NULL) {					// must get rid of old one
			deleteStreamSource(index);
			m_diskManagers[index] = NULL;
		}

		if (m_settings->value(SYNTROSTORE_PARAMS_INUSE).toString() == SYNTRO_PARAMS_FALSE) {
			m_settings->endArray();
			return;											// just needed to clear the entry
		}
		m_sources[index] = new StoreStream();
		m_diskManagers[index] = new StoreDiskManager(m_sources[index]);
		m_sources[index]->load(m_settings);
		m_diskManagers[index]->resumeThread();

		if (m_sources[index]->folderWritable()) {
			m_sources[index]->port = clientAddService(m_sources[index]->streamName(), SERVICETYPE_MULTICAST, false);
			clientSetServiceData(m_sources[index]->port, index);	// record my index in service entry
		}
		else {
			logError(QString("Folder is not writable: %1").arg(m_sources[index]->pathOnly()));
		}
	}
	m_settings->endArray();
}

void StoreClient::deleteStreamSource(int index)
{
	if ((index < 0) || (index >= SYNTROSTORE_MAX_STREAMS)) {
		logWarn(QString("Got deleteStreamSource with out of range index %1").arg(index));
		return;
	}

	if (m_diskManagers[index] == NULL)
		return;												// not currently active

	clientRemoveService(m_sources[index]->port);
	m_diskManagers[index]->exitThread();
	m_diskManagers[index]->wait(5);
	delete m_sources[index];
	m_sources[index] = NULL;
}
