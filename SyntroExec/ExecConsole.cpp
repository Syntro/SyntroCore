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

#include "ExecConsole.h"
#include "SyntroLib.h"
#include "ComponentManager.h"

#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>
#endif


ExecConsole::ExecConsole(QSettings *settings, QObject *parent)
	: QThread(parent)
{
	m_settings = settings;
	SyntroUtils::syntroAppInit(m_settings);

	connect((QCoreApplication *)parent, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));

	m_manager = new ComponentManager(m_settings);
	m_manager->resumeThread();

	connect(this, SIGNAL(loadComponent(int)), m_manager, SLOT(loadComponent(int)), Qt::QueuedConnection);

	for (int i = 0; i < SYNTRO_MAX_COMPONENTSPERDEVICE; i++)
		emit loadComponent(i);

	start();
}

void ExecConsole::aboutToQuit()
{
	m_manager->exitThread();
	delete m_manager;
	SyntroUtils::syntroAppExit();
	for (int i = 0; i < 5; i++) {
		if (wait(1000))
			break;

		printf("Waiting for console thread to finish...\n");
	}
}

void ExecConsole::displayComponents()
{
	Hello *helloObject;
	HELLOENTRY helloEntry;

	helloObject = m_manager->getHelloObject();

	printf("\n%-20s %-20s %-20s %-16s\n",
			"Comp type",
			"Comp name",
			"UID",
			"IP address");
	printf("%-20s %-20s %-20s %-16s\n",
			"---------",
			"---------",
			"---",
			"----------");

	for (int i = 0; i < SYNTRO_MAX_CONNECTEDCOMPONENTS; i++) {
		helloObject->copyHelloEntry(i, &helloEntry);
		if (!helloEntry.inUse)
			continue;										// not in use
		printf("%-20s %-20s %-20s %-16s\n", helloEntry.hello.componentType, helloEntry.hello.componentName,
				qPrintable(SyntroUtils::displayUID(&helloEntry.hello.componentUID)),
				qPrintable(SyntroUtils::displayIPAddr(helloEntry.hello.IPAddr)));
	}
}


void ExecConsole::displayManagedComponents()
{
	ManagerComponent *component;
	int	index;

	printf("\n%-5s %-18s %-17s %-10s %-4s %-12s %-16s\n", 
			"Inst", "App name", "UID", "State", "Mon", "Hello state", "Comp name");
	printf("%-5s %-18s %-17s %-10s %-4s %-12s %-16s\n", 
			"----", "--------", "---", "-----", "---", "-----------", "---------");

	index = 0;
	while ((component = m_manager->getComponent(index++)) != NULL) {
		if (component->processState == "unused")
			continue;

		printf("%-5d %-18s %-17s %-10s %-4s ", 
			component->instance, 
			qPrintable(component->appName), 
			qPrintable(SyntroUtils::displayUID(&component->UID)), 
			qPrintable(component->processState), 
			component->monitored ? "yes" : "no");

		if (component->monitored) {
			printf("%-12s %-16s\n", 
			qPrintable(component->helloStateString), 
			qPrintable(component->componentName));
		} 
		else {
			printf ("\n");
		}
	}
}

void ExecConsole::showHelp()
{
	printf("\nOptions are:\n\n");
	printf("  C - Display active components\n");
	printf("  H - Display this help page\n");
	printf("  M - Display managed components\n");
	printf("  X - Exit\n");
}

void ExecConsole::run()
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
			displayComponents();
			break;

		case 'H':
			showHelp();
			break;

		case 'M':
			displayManagedComponents();
			break;

		case 'X':
			printf("\nExiting\n");
			mustExit = true;
			((QCoreApplication *)parent())->exit();
			break;

		default:
			break;
		}
	}
}

