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
#include "CFSClient.h"
#include "SyntroLib.h"
#include "SyntroRecord.h"
#include "CFSThread.h"

CFSClient::CFSClient(QObject *parent)
	: Endpoint(SYNTROCFS_BGND_INTERVAL, COMPTYPE_CFS),  m_parent(parent)
{
	m_CFSThread = new CFSThread(this);
}

void CFSClient::appClientInit()
{
	m_CFSPort = clientAddService(SYNTRO_STREAMNAME_CFS, SERVICETYPE_E2E, true);
	m_CFSThread->resumeThread();
	return;
}

void CFSClient::appClientExit()
{
	m_CFSThread->exitThread();
}

void CFSClient::appClientBackground()
{

}

CFSThread *CFSClient::getCFSThread()
{
	return m_CFSThread;
}

void CFSClient::appClientReceiveE2E(int servicePort, SYNTRO_EHEAD *message, int length)
{
	if (servicePort != m_CFSPort) {
		logWarn(QString("Message received with incorrect service port %1").arg(servicePort));
		free(message);
		return;
	}
	m_CFSThread->postThreadMessage(SYNTRO_CFS_MESSAGE, length, message);
}



