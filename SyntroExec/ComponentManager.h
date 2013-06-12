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

#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include <QThread>
#include <QProcess>
#include "SyntroLib.h"
#include "SyntroComponentData.h"

class ManagerComponent
{
public:

//	params from .ini

	bool inUse;												// true if being used
	QString appName;										// the Syntro app name
	QString executableDirectory;							// the directory that contains the Syntro app executable
	QString workingDirectory;								// the working directory
	QString adaptor;										// the network adaptor name
	bool consoleMode;										// true if console mode
	QString iniPath;										// path to the .ini file
	bool monitored;											// if monitoring hellos

//	internally generated vars

	int instance;											// the components instance
	QProcess process;										// the actual process
	QString processState;									// current state of the process
	qint64 timeStarted;										// last time this component was started
	SYNTRO_UID UID;											// its UID

//	these related to received hellos

	bool helloSeen;											// if a hello has ever been seen for this component
	qint64 lastHelloTime;									// last time a hello was received
	QString helloStateString;								// string version of hello state
	QString componentName;									// the actual name of the component
	QString IPAddressString;								// the IP address of the component
};

class ComponentManager : public SyntroThread
{
	Q_OBJECT

public:
	ComponentManager(QSettings *settings);
	~ComponentManager() {}
	void exitThread();

	Hello *getHelloObject();								// returns the hello object
	ManagerComponent *getComponent(int index);				// gets a component object or null if out of range

	QMutex m_lock;											// for access to the m_component table

	SyntroComponentData m_componentData;

public slots:
	void loadComponent(int index);							// load specified component

signals:
	void updateExecStatus(ManagerComponent *managerComponent);

protected:
	void initThread();
	bool processMessage(SyntroThreadMsg* pMsg);

private:
	QSettings *m_settings;
	Hello *m_hello;
	int m_timer;

	ManagerComponent m_components[SYNTRO_MAX_COMPONENTSPERDEVICE]; // the component array		
	QString m_extension;									// OS-dependent executable extension
	qint64 m_startTime;										// used to time the period looking for old apps
	bool m_startMode;										// if in start mode

	void killComponents();
	void checkHello(ManagerComponent *component);
	void startModeKillAll();								// kill all running components at startup
	void findAndKillProcess(QString processName);			// finds and kills processes at startup
	void managerBackground();
	void startComponent(ManagerComponent *component);
};

#endif // COMPONENTMANAGER_H
