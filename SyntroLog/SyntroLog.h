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

#ifndef SYNTROLOG_H
#define SYNTROLOG_H

#include <QMainWindow>
#include "ui_syntrolog.h"
#include "LogClient.h"

#define PRODUCT_TYPE "SyntroLog"

class SyntroLog : public QMainWindow
{
	Q_OBJECT

public:
	SyntroLog(QSettings *settings, QWidget *parent = 0);
	
public slots:
	void newLogMsg(QByteArray bulkMsg);
	void activeClientUpdate(int count);
	void onSave();
	void onBasicSetup();
	void onAbout();

protected:
	void closeEvent(QCloseEvent *event);
	void timerEvent(QTimerEvent *event);
	
private:
	void saveWindowState();
	void restoreWindowState();
	void initStatusBar();
	void initGrid();
	void parseMsgQueue();
	void addMessage(QString msg);
	int findRowInsertPosition(QString timestamp);

	Ui::SyntroLogClass ui;

	QSettings *m_settings;
	LogClient *m_client;
	int m_timer;
	QQueue<QByteArray> m_logQ;
	QMutex m_logMutex;

	int m_activeClientCount;
	QMutex m_activeClientMutex;

	QLabel *m_controlStatus;
	QLabel *m_activeClientStatus;

	QString m_savePath;
};

#endif // SYNTROLOG_H
