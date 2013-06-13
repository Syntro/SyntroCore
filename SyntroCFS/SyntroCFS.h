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

#ifndef SYNTROCFS_H
#define SYNTROCFS_H

#include <QMainWindow>
#include <qtablewidget.h>
#include "ui_SyntroCFS.h"
#include "SyntroLib.h"
#include "SyntroCFSClient.h"

#define	PRODUCT_TYPE	"SyntroCFS"

#define	SYNTROCFS_MAX_FILES		1024						// max files open at one time

// SyntroCFS specific setting keys

#define	SYNTRO_CFS_STORE_PATH				"storePath"	

//	The SyntroCFS state information referenced by a stream handle

typedef struct
{
public:
	bool inUse;												// true if this represents an open stream
	int	storeHandle;										// the index of this entry
	int clientHandle;										// the client's handle
	SYNTRO_UID clientUID;									// the uid of the client app
	int clientPort;											// the client port
	unsigned int fileIndex;									// the last requested file record index
	QString filePath;										// path of the file
	QString indexPath;										// if its structured
	bool structured;										// true if structured, false if flat
	unsigned int fileLength;								// the length of the current file in units of records/blocks
	qint64 lastKeepalive;									// last time a keepalive was received
	qint64 rxBytes;											// total bytes received for this file
	qint64 txBytes;											// total bytes sent for this file
	qint64 lastStatusEmit;									// the time that the last status was emitted

	int blockSize;											// size of blocks for flat file transfers
} SYNTROCFS_STATE;


//	The CFS message

#define	SYNTRO_CFS_MESSAGE					(SYNTRO_MSTART + 0)	// the message used to pass CFS messages around

//	Timer intervals

#define	SYNTROCFS_BGND_INTERVAL				(SYNTRO_CLOCKS_PER_SEC / 100)
#define	SYNTROCFS_DM_INTERVAL				(SYNTRO_CLOCKS_PER_SEC * 10)

class SyntroCFS : public QMainWindow
{
	Q_OBJECT

public:
	SyntroCFS(QSettings *settings, QWidget *parent = 0);
	~SyntroCFS() {};

public slots:
	void newStatus(int handle, SYNTROCFS_STATE *CFSState);
	void onAbout();
	void onBasicSetup();
	void onConfiguration();

protected:
	void closeEvent(QCloseEvent *event);
	void timerEvent(QTimerEvent *event);

private:
	void saveWindowState();
	void restoreWindowState();
	void initStatusBar();
	void initDisplayStats();
	
	Ui::CSyntroCFSClass ui;
	QLabel *m_controlStatus;
	QTableWidget *m_fileTable;
	QSettings *m_settings;
	SyntroCFSClient *m_client;
	int	m_timerId;
};

#endif // SYNTROCFS_H
