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

#ifndef SYNTROCONTROL_H
#define SYNTROCONTROL_H

#include <QMainWindow>
#include "ui_SyntroControl.h"
#include "SyntroLib.h"
#include "SyntroServer.h"
#include "DirectoryDialog.h"
#include "MulticastDialog.h"
#include "HelloDialog.h"

#define	PRODUCT_TYPE		COMPTYPE_CONTROL


class SyntroControl : public QMainWindow
{
	Q_OBJECT

public:
	SyntroControl(QSettings *settings, QWidget *parent = 0);
	~SyntroControl() {}

public slots:
	void onAbout();
	void onDirectory();
	void onBasicSetup();
	void onHello();
	void onMulticast();
	void UpdateSyntroStatusBox(SS_COMPONENT *syntroComponent);
	void serverMulticastUpdate(qint64 in, unsigned inRate, qint64 out, unsigned outRate);
	void serverE2EUpdate(qint64 in, unsigned inRate, qint64 out, unsigned outRate);

protected:
	DirectoryDialog *m_directoryDlg;
	HelloDialog *m_helloDlg;
	MulticastDialog *m_multicastDlg;
	QSettings *m_settings;
	QLabel *m_serverE2EStatus;
	QLabel *m_serverMulticastStatus;
	void timerEvent(QTimerEvent *event);

private:
	void closeEvent(QCloseEvent * event);
	void saveWindowState();
	void restoreWindowState();

	Ui::SyntroControlClass ui;
	SyntroServer *m_server;

	int m_timer;
	bool m_helloConnected;
};

#endif // SYNTROCONTROL_H
