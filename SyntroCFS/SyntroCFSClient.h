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

#ifndef SYNTROCFSCLIENT_H
#define SYNTROCFSCLIENT_H

#include "SyntroLib.h"

class CFSThread;

class SyntroCFSClient : public Endpoint
{
	Q_OBJECT

friend class CFSThread;

public:
	SyntroCFSClient(QObject *parent, QSettings *settings);
	~SyntroCFSClient() {}

	CFSThread *getCFSThread();

protected:
	void appClientInit();
	void appClientExit();
	void appClientReceiveE2E(int servicePort, SYNTRO_EHEAD *message, int length);

	int m_CFSPort;										// the local service port assigned to CFS

private:

	QObject	*m_parent;
	CFSThread *m_CFSThread;								// the worker thread

};

#endif // SYNTROCFSCLIENT_H


