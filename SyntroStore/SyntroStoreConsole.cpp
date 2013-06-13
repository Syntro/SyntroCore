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

#include "SyntroStoreConsole.h"
#include "SyntroLib.h"
#include "SyntroStore.h"
#include "StoreClient.h"

#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>
#endif


SyntroStoreConsole::SyntroStoreConsole(QSettings *settings, QObject *parent)
	: QThread(parent), m_settings(settings)
{
	SyntroUtils::syntroAppInit(m_settings);
	m_client = new StoreClient(this, m_settings);
	m_client->resumeThread();
	connect(this, SIGNAL(refreshStreamSource(int)), m_client, SLOT(refreshStreamSource(int)), Qt::QueuedConnection); 
	for (int index = 0; index < SYNTROSTORE_MAX_STREAMS; index++)
		emit refreshStreamSource(index);

	start();
}

SyntroStoreConsole::~SyntroStoreConsole()
{
}

void SyntroStoreConsole::showHelp()
{
	printf("\n\nOptions are:\n\n");
	printf("  C - Display record and byte counts\n");
	printf("  S - Display status\n");
	printf("  X - Exit\n");
}

void SyntroStoreConsole::showStatus()
{
	printf("\n\nSyntroControl status is: %s\n", qPrintable(m_client->getLinkState()));
}

void SyntroStoreConsole::showCounts()
{
	printf("\n\n%-15s %-12s %-16s %s", "Service", "RX records", "RX bytes", "Active File");
	printf("\n%-15s %-12s %-16s %s", "-------", "----------", "--------", "------------------------");

	StoreStream *storeStream;

	for (int i = 0; i < SYNTROSTORE_MAX_STREAMS; i++) {
		m_client->m_lock.lock();
		if (m_client->m_sources[i] == NULL) {
			m_client->m_lock.unlock();
			continue;
		}
		storeStream = m_client->m_sources[i];
		printf("\n%-15s %-12s %-16s %s",
			qPrintable(storeStream->streamName()),
			qPrintable(QString::number(storeStream->rxTotalRecords())),
			qPrintable(QString::number(storeStream->rxTotalBytes())),
			qPrintable(storeStream->currentFile()));			
		m_client->m_lock.unlock();
	}
	printf("\n");
}

void SyntroStoreConsole::run()
{
	bool mustExit;

#ifndef WIN32
        struct termios	ctty;

        tcgetattr(fileno(stdout), &ctty);
        ctty.c_lflag &= ~(ICANON);
        tcsetattr(fileno(stdout), TCSANOW, &ctty);
#endif
	mustExit = false;

	while (!mustExit) {
		printf("\nEnter option: ");

#ifdef WIN32
		switch (toupper(_getch()))
#else
        switch (toupper(getchar()))
#endif		
		{
		case 'C':
			showCounts();
			break;

		case 'S':
			showStatus();
			break;

		case 'H':
			showHelp();
			break;

		case 'X':
			printf("\nExiting\n");
			m_client->exitThread();
			mustExit = true;
			SyntroUtils::syntroAppExit();
			((QCoreApplication *)parent())->exit();
			break;

		default:
			break;;
		}
	}
}

