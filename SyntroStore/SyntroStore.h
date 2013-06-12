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

#ifndef SYNTROSTORE_H
#define SYNTROSTORE_H

#include <QMainWindow>
#include <qtablewidget.h>
#include "ui_SyntroStore.h"
#include "SyntroLib.h"
#include "StoreStreamDlg.h"

#define	PRODUCT_TYPE	"SyntroStore"

#define	SYNTROSTORE_MAX_STREAMS		(SYNTRO_MAX_SERVICESPERCOMPONENT / 2)

// SyntroStore specific setting keys

#define	SYNTROSTORE_MAXAGE					"maxAge"

#define	SYNTROSTORE_PARAMS_STREAM_SOURCES	"Streams"
#define	SYNTROSTORE_PARAMS_INUSE			"inUse"
#define	SYNTROSTORE_PARAMS_STREAM_SOURCE	"stream"
#define	SYNTROSTORE_PARAMS_PATH				"storePath"	
#define SYNTROSTORE_PARAMS_CREATE_SUBFOLDER	"createSubFolder"
#define	SYNTROSTORE_PARAMS_FORMAT			"storeFormat"
#define SYNTROSTORE_PARAMS_ROTATION_POLICY	"rotationPolicy"
#define SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS	"rotationTimeUnits"
#define SYNTROSTORE_PARAMS_ROTATION_TIME	"rotationTime"
#define SYNTROSTORE_PARAMS_ROTATION_SIZE	"rotationSize"

//	magic strings used in the .ini file

#define	SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS_HOURS	"hours"
#define	SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS_MINUTES	"minutes"
#define SYNTROSTORE_PARAMS_ROTATION_POLICY_TIME			"time"
#define SYNTROSTORE_PARAMS_ROTATION_POLICY_SIZE			"size"
#define SYNTROSTORE_PARAMS_ROTATION_POLICY_ANY			"any"


//	Display columns

#define	SYNTROSTORE_COL_CONFIG			0					// configure entry
#define	SYNTROSTORE_COL_INUSE			1					// entry in use
#define	SYNTROSTORE_COL_STREAM			2					// stream name
#define	SYNTROSTORE_COL_TOTALRECS		3					// total records
#define	SYNTROSTORE_COL_TOTALBYTES		4					// total bytes
#define	SYNTROSTORE_COL_FILERECS		5					// file records
#define	SYNTROSTORE_COL_FILEBYTES		6					// file bytes
#define	SYNTROSTORE_COL_FILE			7					// name of current file

#define SYNTROSTORE_COL_COUNT			8					// number of columns

//	Timer intervals

#define	SYNTROSTORE_BGND_INTERVAL		(SYNTRO_CLOCKS_PER_SEC/10)
#define	SYNTROSTORE_DM_INTERVAL			(SYNTRO_CLOCKS_PER_SEC * 10)

class StoreClient;

class StoreButton : public QPushButton
{
	Q_OBJECT

public:
	StoreButton(const QString& text, QWidget *parent, int id);

public slots:
	void originalClicked(bool);

signals:
	void buttonClicked(int);

private:
	int m_id;
};

class StoreCheckBox : public QCheckBox
{
	Q_OBJECT

public:
	StoreCheckBox(QWidget *parent, int id);

public slots:
	void originalClicked(bool);

signals:
	void boxClicked(bool, int);

private:
	int m_id;
};


class SyntroStore : public QMainWindow
{
	Q_OBJECT

public:
	SyntroStore(QSettings *settings, QWidget *parent = 0);
	~SyntroStore() {};

public slots:
	void onAbout();
	void onBasicSetup();
	void boxClicked(bool, int);
	void buttonClicked(int);

signals:
	void refreshStreamSource(int index);

protected:
	void closeEvent(QCloseEvent *event);
	void timerEvent(QTimerEvent *event);

private:
	void saveWindowState();
	void restoreWindowState();
	void initStatusBar();
	void initDisplayStats();
	void displayStreamDetails(int index);
	
	Ui::CSyntroStoreClass ui;
	QLabel *m_controlStatus;
	QTableWidget *m_rxStreamTable;
	QSettings *m_settings;
	StoreClient *m_client;
	int	m_timerId;

	StoreCheckBox *m_useBox[SYNTROSTORE_MAX_STREAMS];

	StoreStreamDlg *m_storeStreamDlg;

	bool m_startingUp;
};

#endif // SYNTROSTORE_H
