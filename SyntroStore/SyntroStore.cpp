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

#include "SyntroStore.h"
#include "SyntroAboutDlg.h"
#include "StoreClient.h"
#include "BasicSetupDlg.h"

#define CELL_HEIGHT_PAD 6

SyntroStore::SyntroStore(QSettings *settings, QWidget *parent)
	: QMainWindow(parent), m_settings(settings)
{
	m_startingUp = true;
	ui.setupUi(this);

	m_storeStreamDlg = NULL;

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
	connect(ui.actionBasicSetup, SIGNAL(triggered()), this, SLOT(onBasicSetup()));

	initStatusBar();
	initDisplayStats();
	restoreWindowState();

	m_timerId = startTimer(2000);

	SyntroUtils::syntroAppInit(m_settings);
	m_client = new StoreClient(this, m_settings);
	setWindowTitle(QString("%1 - %2")
		.arg(m_settings->value(SYNTRO_PARAMS_APPNAME).toString())
		.arg(m_settings->value(SYNTRO_PARAMS_COMPTYPE).toString()));
	
	connect(this, SIGNAL(refreshStreamSource(int)), m_client, SLOT(refreshStreamSource(int)), Qt::QueuedConnection); 
	m_client->resumeThread();

	m_settings->beginReadArray(SYNTROSTORE_PARAMS_STREAM_SOURCES);
	for (int index = 0; index < SYNTROSTORE_MAX_STREAMS; index++) {
		m_settings->setArrayIndex(index);

		m_rxStreamTable->item(index, SYNTROSTORE_COL_STREAM)->setText(m_settings->value(SYNTROSTORE_PARAMS_STREAM_SOURCE).toString());
		if (m_settings->value(SYNTROSTORE_PARAMS_INUSE).toString() == SYNTRO_PARAMS_TRUE)
			m_useBox[index]->setCheckState(Qt::Checked);
		else
			m_useBox[index]->setCheckState(Qt::Unchecked);
	}
	m_settings->endArray();

	for (int index = 0; index < SYNTROSTORE_MAX_STREAMS; index++)
		emit refreshStreamSource(index);

	m_startingUp = false;
}

void SyntroStore::closeEvent(QCloseEvent *)
{
	killTimer(m_timerId);

	if (m_client) {
		m_client->exitThread();
	}

	m_settings->beginWriteArray(SYNTROSTORE_PARAMS_STREAM_SOURCES);
	for (int index = 0; index < SYNTROSTORE_MAX_STREAMS; index++) {
		m_settings->setArrayIndex(index);

		if (m_useBox[index]->checkState() == Qt::Checked)
			m_settings->setValue(SYNTROSTORE_PARAMS_INUSE, SYNTRO_PARAMS_TRUE);
		else
			m_settings->setValue(SYNTROSTORE_PARAMS_INUSE, SYNTRO_PARAMS_FALSE);
	}
	m_settings->endArray();

	saveWindowState();
	SyntroUtils::syntroAppExit();
}

void SyntroStore::displayStreamDetails(int index)
{
	m_settings->beginReadArray(SYNTROSTORE_PARAMS_STREAM_SOURCES);
	m_settings->setArrayIndex(index);

	m_rxStreamTable->item(index, SYNTROSTORE_COL_STREAM)->setText(m_settings->value(SYNTROSTORE_PARAMS_STREAM_SOURCE).toString());

	m_settings->endArray();
}

void SyntroStore::timerEvent(QTimerEvent *)
{
	StoreStream *storeStream;

	for (int i = 0; i < SYNTROSTORE_MAX_STREAMS; i++) {
		m_client->m_lock.lock();
		if (m_client->m_sources[i] == NULL) {
			m_client->m_lock.unlock();
			continue;
		}
		storeStream = m_client->m_sources[i];
		m_rxStreamTable->item(i, SYNTROSTORE_COL_TOTALRECS)->setText(QString::number(storeStream->rxTotalRecords()));
		m_rxStreamTable->item(i, SYNTROSTORE_COL_TOTALBYTES)->setText(QString::number(storeStream->rxTotalBytes()));
		m_rxStreamTable->item(i, SYNTROSTORE_COL_FILERECS)->setText(QString::number(storeStream->rxRecords()));
		m_rxStreamTable->item(i, SYNTROSTORE_COL_FILEBYTES)->setText(QString::number(storeStream->rxBytes()));
		m_rxStreamTable->item(i, SYNTROSTORE_COL_FILE)->setText(storeStream->currentFile());
		m_client->m_lock.unlock();
	}

	m_controlStatus->setText(m_client->getLinkState());
}

void SyntroStore::initDisplayStats()
{
	QTableWidgetItem *item;

	int cellHeight = fontMetrics().lineSpacing() + CELL_HEIGHT_PAD;
	
	m_rxStreamTable = new QTableWidget(this);

	m_rxStreamTable->setColumnCount(SYNTROSTORE_COL_COUNT);
	m_rxStreamTable->setColumnWidth(SYNTROSTORE_COL_CONFIG, 80);
	m_rxStreamTable->setColumnWidth(SYNTROSTORE_COL_INUSE, 60);
	m_rxStreamTable->setColumnWidth(SYNTROSTORE_COL_STREAM, 140);
	m_rxStreamTable->setColumnWidth(SYNTROSTORE_COL_TOTALRECS, 80);
	m_rxStreamTable->setColumnWidth(SYNTROSTORE_COL_TOTALBYTES, 100);
	m_rxStreamTable->setColumnWidth(SYNTROSTORE_COL_FILERECS, 80);
	m_rxStreamTable->setColumnWidth(SYNTROSTORE_COL_FILEBYTES, 100);
	m_rxStreamTable->setColumnWidth(SYNTROSTORE_COL_FILE, 200);

	m_rxStreamTable->verticalHeader()->setDefaultSectionSize(cellHeight);

    m_rxStreamTable->setHorizontalHeaderLabels(QStringList() << tr("") << tr("In use") << tr("Stream") 
			<< tr("Total Recs") << tr("Total Bytes") 
			<< tr("File Recs") << tr("File Bytes") << tr("Current file path"));

    m_rxStreamTable->setSelectionMode(QAbstractItemView::NoSelection);
 
	for (int row = 0; row < SYNTROSTORE_MAX_STREAMS; row++) {
		m_rxStreamTable->insertRow(row);
		m_rxStreamTable->setRowHeight(row, 20);
		m_rxStreamTable->setContentsMargins(5, 5, 5, 5);

		StoreButton *button = new StoreButton("Configure", this, row);
		m_rxStreamTable->setCellWidget(row, SYNTROSTORE_COL_CONFIG, button);
		connect(button, SIGNAL(buttonClicked(int)), this, SLOT(buttonClicked(int)));

		m_useBox[row] = new StoreCheckBox(m_rxStreamTable, row);
		m_rxStreamTable->setCellWidget(row, SYNTROSTORE_COL_INUSE, m_useBox[row]);
		connect(m_useBox[row], SIGNAL(boxClicked(bool, int)), this, SLOT(boxClicked(bool, int)));
		
		for (int col = 2; col < SYNTROSTORE_COL_COUNT; col++) {
			item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignLeft | Qt::AlignBottom);
			item->setFlags(Qt::ItemIsEnabled);
			item->setText("");
			m_rxStreamTable->setItem(row, col, item);
		}
	}
	setCentralWidget(m_rxStreamTable);
}

void SyntroStore::initStatusBar()
{
	m_controlStatus = new QLabel(this);
	m_controlStatus->setAlignment(Qt::AlignLeft);
	m_controlStatus->setText("");
	ui.statusBar->addWidget(m_controlStatus, 1);
}

void SyntroStore::saveWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("Window");
		m_settings->setValue("Geometry", saveGeometry());
		m_settings->setValue("State", saveState());

		m_settings->beginWriteArray("Grid");
		for (int i = 0; i < m_rxStreamTable->columnCount(); i++) {
			m_settings->setArrayIndex(i);
			m_settings->setValue("columnWidth", m_rxStreamTable->columnWidth(i));
		}
		m_settings->endArray();

		m_settings->endGroup();
	}
}

void SyntroStore::restoreWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("Window");
		restoreGeometry(m_settings->value("Geometry").toByteArray());
		restoreState(m_settings->value("State").toByteArray());

		int count = m_settings->beginReadArray("Grid");
		for (int i = 0; i < count && i < m_rxStreamTable->columnCount(); i++) {
			m_settings->setArrayIndex(i);
			int width = m_settings->value("columnWidth").toInt();

			if (width > 0)
				m_rxStreamTable->setColumnWidth(i, width);
		}
		m_settings->endArray();

		m_settings->endGroup();
	}
}

void SyntroStore::onAbout()
{
	SyntroAbout *dlg = new SyntroAbout(this, m_settings);
	dlg->show();
}

void SyntroStore::onBasicSetup()
{
	BasicSetupDlg *dlg = new BasicSetupDlg(this, m_settings);
	dlg->show();
}

void SyntroStore::buttonClicked(int buttonId)
{
	StoreStreamDlg *dlg = new StoreStreamDlg(this, m_settings, buttonId);
	if (dlg->exec()) {										// need to update client with changes
		TRACE1("Refreshing source index %d", buttonId);
		displayStreamDetails(buttonId);
		emit refreshStreamSource(buttonId);
	}
}

void SyntroStore::boxClicked(bool state, int boxId)
{
	if (m_startingUp)
		return;												// avoids thrashing when loading config
	m_settings->beginWriteArray(SYNTROSTORE_PARAMS_STREAM_SOURCES);
	m_settings->setArrayIndex(boxId);

	if (m_useBox[boxId]->checkState() == Qt::Checked)
		m_settings->setValue(SYNTROSTORE_PARAMS_INUSE, SYNTRO_PARAMS_TRUE);
	else
		m_settings->setValue(SYNTROSTORE_PARAMS_INUSE, SYNTRO_PARAMS_FALSE);
	m_settings->endArray();
	if (!state) {											// disabled so zero stats
		m_rxStreamTable->item(boxId, SYNTROSTORE_COL_TOTALRECS)->setText("");
		m_rxStreamTable->item(boxId, SYNTROSTORE_COL_TOTALBYTES)->setText("");
		m_rxStreamTable->item(boxId, SYNTROSTORE_COL_FILERECS)->setText("");
		m_rxStreamTable->item(boxId, SYNTROSTORE_COL_FILEBYTES)->setText("");
		m_rxStreamTable->item(boxId, SYNTROSTORE_COL_FILE)->setText("");
	}
	emit refreshStreamSource(boxId);
}


//----------------------------------------------------------
//
// StoreButton functions

StoreButton::StoreButton(const QString& text, QWidget *parent, int id)
	: QPushButton(text, parent) 
{
	m_id = id;
	connect(this, SIGNAL(clicked(bool)), this, SLOT(originalClicked(bool)));
};

void StoreButton::originalClicked(bool )
{
	emit buttonClicked(m_id);
}

//----------------------------------------------------------
//
// StoreCheckBox functions

StoreCheckBox::StoreCheckBox(QWidget *parent, int id)
	: QCheckBox(parent) 
{
	m_id = id;
	connect(this, SIGNAL(clicked(bool)), this, SLOT(originalClicked(bool)));
};

void StoreCheckBox::originalClicked(bool state)
{
	emit boxClicked(state, m_id);
}