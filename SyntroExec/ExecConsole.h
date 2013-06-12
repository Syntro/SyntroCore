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

#ifndef EXECCONSOLE_H
#define EXECCONSOLE_H

#include <QThread>
#include <QSettings>

class	ComponentManager;

class ExecConsole : public QThread
{
	Q_OBJECT

public:
	ExecConsole(QSettings *settings, QObject *parent);
	~ExecConsole() {}

public slots:
	void aboutToQuit();

signals:
	void loadComponent(int);								// load specified component

protected:
	void run();

private:
	void displayComponents();
	void displayManagedComponents();
	void showHelp();

	QSettings *m_settings;
	ComponentManager *m_manager;
};

#endif // EXECCONSOLE_H
