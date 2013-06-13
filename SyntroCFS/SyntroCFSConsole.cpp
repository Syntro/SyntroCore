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

#include "SyntroCFSConsole.h"
#include "SyntroLib.h"
#include "SyntroCFS.h"
#include "SyntroCFSClient.h"
#include "CFSThread.h"

#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>
#endif


SyntroCFSConsole::SyntroCFSConsole(QSettings *settings, QObject *parent)
	: QThread(parent), m_settings(settings)
{
	SyntroUtils::syntroAppInit(m_settings);
	m_client = new SyntroCFSClient(this, m_settings);
	m_client->resumeThread();
	
	start();
}

SyntroCFSConsole::~SyntroCFSConsole()
{
}

void SyntroCFSConsole::showHelp()
{
	printf("\n\nOptions are:\n\n");
	printf("  C - Display record and byte counts\n");
	printf("  S - Display status\n");
	printf("  X - Exit\n");
}

void SyntroCFSConsole::showStatus()
{
	printf("\n\nSyntroControl status is: %s\n", qPrintable(m_client->getLinkState()));
}

void SyntroCFSConsole::showCounts()
{
	SYNTROCFS_STATE *CFSState;

	printf("\n\n%-50s %-12s %-16s %s", "Active file", "Client UID", "RX bytes", "TX bytes");
	printf("\n%-50s %-12s %-16s %s", "-----------", "----------", "--------", "--------");

	QMutexLocker locker(&(m_client->getCFSThread()->m_lock));// make sure it's safe

	CFSState = m_client->getCFSThread()->m_cfsState;
	for (int i = 0; i < SYNTROCFS_MAX_FILES; i++, CFSState++) {
		if (!CFSState->inUse)
			continue;
	printf("\n%-50s %-12s %-16s %s", 
		qPrintable(CFSState->filePath), qPrintable(SyntroUtils::displayUID(&CFSState->clientUID)),
		qPrintable(QString::number(CFSState->rxBytes)), qPrintable(QString::number(CFSState->txBytes)));
	}
	printf("\n");
}

void SyntroCFSConsole::run()
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
			SyntroUtils::syntroAppExit();
			mustExit = true;
			((QCoreApplication *)parent())->exit();
			break;

		default:
			break;;
		}
	}
}

