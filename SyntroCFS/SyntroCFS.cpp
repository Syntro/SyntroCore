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

#include "SyntroCFS.h"
#include "SyntroAboutDlg.h"
#include "CFSThread.h"
#include "BasicSetupDlg.h"
#include "ConfigurationDlg.h"

#define CELL_HEIGHT_PAD 6

SyntroCFS::SyntroCFS(QSettings *settings, QWidget *parent)
	: QMainWindow(parent), m_settings(settings)
{
	ui.setupUi(this);

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
	connect(ui.actionBasicSetup, SIGNAL(triggered()), this, SLOT(onBasicSetup()));
	connect(ui.actionConfiguration, SIGNAL(triggered()), this, SLOT(onConfiguration()));

	initStatusBar();
	initDisplayStats();
	restoreWindowState();

	m_timerId = startTimer(2000);

	SyntroUtils::syntroAppInit(m_settings);
	m_client = new SyntroCFSClient(this, m_settings);
	setWindowTitle(QString("%1 - %2")
		.arg(m_settings->value(SYNTRO_PARAMS_APPNAME).toString())
		.arg(m_settings->value(SYNTRO_PARAMS_COMPTYPE).toString()));
	connect(m_client->getCFSThread(), SIGNAL(newStatus(int, SYNTROCFS_STATE *)), this, SLOT(newStatus(int, SYNTROCFS_STATE *)), Qt::QueuedConnection);
	m_client->resumeThread();
}

void SyntroCFS::closeEvent(QCloseEvent *)
{
	killTimer(m_timerId);
	m_client->exitThread();
	saveWindowState();
	SyntroUtils::syntroAppExit();
}

void SyntroCFS::timerEvent(QTimerEvent *)
{

	m_controlStatus->setText(m_client->getLinkState());
}


void SyntroCFS::initDisplayStats()
{
	int cellHeight = fontMetrics().lineSpacing() + CELL_HEIGHT_PAD;

	m_fileTable = new QTableWidget(this);
	m_fileTable->setColumnCount(4);
	m_fileTable->setColumnWidth(0, 300);
	m_fileTable->setColumnWidth(1, 140);
	m_fileTable->setColumnWidth(2, 120);
	m_fileTable->setColumnWidth(3, 120);
	m_fileTable->verticalHeader()->setDefaultSectionSize(cellHeight);
	m_fileTable->verticalHeader()->stretchLastSection();

    m_fileTable->setHorizontalHeaderLabels(QStringList() << tr("File path") 
			<< tr("Client UID") << tr("Total bytes in") 
			<< tr("Total bytes out"));

    m_fileTable->setSelectionMode(QAbstractItemView::NoSelection);


	for (int row = 0; row < SYNTROCFS_MAX_FILES; row++) {
		m_fileTable->insertRow(row);
		m_fileTable->setRowHeight(row, 20);

		for (int col = 0; col < 4; col++) {
			QTableWidgetItem *item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignLeft | Qt::AlignBottom);
			item->setFlags(Qt::ItemIsEnabled);

			if (col == 0)
				item->setText("...");
			else
				item->setText("");

			m_fileTable->setItem(row, col, item);
		}
	}

	setCentralWidget(m_fileTable);
}

void SyntroCFS::newStatus(int handle, SYNTROCFS_STATE *CFSState)
{
	QMutexLocker locker(&(m_client->getCFSThread()->m_lock));// make sure it's safe

	if ((handle < 0) || (handle>= m_fileTable->rowCount())) {
		logWarn(QString("Got out of range handle on newStatus %1").arg(handle));
		return;
	}
	if (!CFSState->inUse) {
		(m_fileTable->item(handle, 0))->setText("...");
		(m_fileTable->item(handle, 1))->setText("");
		(m_fileTable->item(handle, 2))->setText("");
		(m_fileTable->item(handle, 3))->setText("");
		return;
	}
	(m_fileTable->item(handle, 0))->setText(CFSState->filePath);
	(m_fileTable->item(handle, 1))->setText(SyntroUtils::displayUID(&CFSState->clientUID));
	(m_fileTable->item(handle, 2))->setText(QString::number(CFSState->rxBytes));
	(m_fileTable->item(handle, 3))->setText(QString::number(CFSState->txBytes));
}

void SyntroCFS::initStatusBar()
{
	m_controlStatus = new QLabel(this);
	m_controlStatus->setAlignment(Qt::AlignLeft);
	m_controlStatus->setText("");
	ui.statusBar->addWidget(m_controlStatus, 1);
}

void SyntroCFS::saveWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("Window");
		m_settings->setValue("Geometry", saveGeometry());
		m_settings->setValue("State", saveState());

		m_settings->beginWriteArray("Grid");
		for (int i = 0; i < m_fileTable->columnCount(); i++) {
			m_settings->setArrayIndex(i);
			m_settings->setValue("columnWidth", m_fileTable->columnWidth(i));
		}
		m_settings->endArray();

		m_settings->endGroup();
	}
}

void SyntroCFS::restoreWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("Window");
		restoreGeometry(m_settings->value("Geometry").toByteArray());
		restoreState(m_settings->value("State").toByteArray());

		int count = m_settings->beginReadArray("Grid");
		for (int i = 0; i < count && i < m_fileTable->columnCount(); i++) {
			m_settings->setArrayIndex(i);
			int width = m_settings->value("columnWidth").toInt();

			if (width > 0)
				m_fileTable->setColumnWidth(i, width);
		}
		m_settings->endArray();

		m_settings->endGroup();
	}
}

void SyntroCFS::onAbout()
{
	SyntroAbout *dlg = new SyntroAbout(this, m_settings);
	dlg->show();
}

void SyntroCFS::onBasicSetup()
{
	BasicSetupDlg *dlg = new BasicSetupDlg(this, m_settings);
	dlg->show();
}

void SyntroCFS::onConfiguration()
{
	QMessageBox msgBox;

	ConfigurationDlg *dlg = new ConfigurationDlg(this, m_settings);
	if (dlg->exec()) {										// need to update client with changes
		msgBox.setText("The component must be restarted for these changes to take effect");
		msgBox.exec();
	}
}
