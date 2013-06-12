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

#include "SyntroControl.h"
#include "SyntroAboutDlg.h"
#include "ControlSetupDlg.h"

#define DEFAULT_ROW_HEIGHT 20


SyntroControl::SyntroControl(QSettings *settings, QWidget *parent)
	: QMainWindow(parent), m_settings(settings)
{
	ui.setupUi(this);

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionDirectory, SIGNAL(triggered()), this, SLOT(onDirectory()));
	connect(ui.actionHello, SIGNAL(triggered()), this, SLOT(onHello()));
	connect(ui.actionMulticast, SIGNAL(triggered()), this, SLOT(onMulticast()));
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
	connect(ui.actionBasicSetup, SIGNAL(triggered()), this, SLOT(onBasicSetup()));

    ui.m_table->setColumnCount(5);

    ui.m_table->setColumnWidth(0, 120);
    ui.m_table->setColumnWidth(1, 120);
     ui.m_table->setColumnWidth(2, 120);
    ui.m_table->setColumnWidth(3, 100);
	ui.m_table->setColumnWidth(4, 80);

    ui.m_table->setHorizontalHeaderLabels(
                QStringList() << tr("App name") << tr("Component name")
                << tr("Unique ID") << tr("IP Address") << tr("HB interval"));


    ui.m_table->setSelectionMode(QAbstractItemView::NoSelection);

	for (int row = 0; row < SYNTRO_MAX_CONNECTEDCOMPONENTS; row++) {
		ui.m_table->insertRow(row);
		ui.m_table->setRowHeight(row, DEFAULT_ROW_HEIGHT);

		for (int col = 0; col < 6; col++) {
			QTableWidgetItem *item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignLeft | Qt::AlignBottom);
			item->setFlags(Qt::ItemIsEnabled);
			if (col == 0)
				item->setText("...");
			else
				item->setText("");

			ui.m_table->setItem(row, col, item);
		}
	}
 
    setCentralWidget(ui.m_table);

	m_serverMulticastStatus = new QLabel(this);
	m_serverMulticastStatus->setAlignment(Qt::AlignLeft);
	m_serverMulticastStatus->setText("");
	ui.statusBar->addWidget(m_serverMulticastStatus, 1);

 	m_serverE2EStatus = new QLabel(this);
	m_serverE2EStatus->setAlignment(Qt::AlignLeft);
	m_serverE2EStatus->setText("");
	ui.statusBar->addWidget(m_serverE2EStatus, 1);

  //  setMinimumSize(600, 120);

	restoreWindowState();

	m_directoryDlg = new DirectoryDialog(this);
	m_directoryDlg->setModal(false);

	m_helloDlg = new HelloDialog(this);
	m_helloDlg->setModal(false);

	m_multicastDlg = new MulticastDialog(this);
	m_multicastDlg->setModal(false);

	setWindowTitle(QString("%1 - %2")
		.arg(m_settings->value(SYNTRO_PARAMS_APPNAME).toString())
		.arg(m_settings->value(SYNTRO_PARAMS_APPTYPE).toString()));

	syntroAppInit(m_settings);
	m_server = new SyntroServer(settings);

	m_multicastDlg->m_multicastMgr = &(m_server->m_multicastManager);
	m_directoryDlg->m_directoryMgr = &(m_server->m_dirManager);

	connect(m_server, SIGNAL(UpdateSyntroStatusBox(SS_COMPONENT *)), this, SLOT(UpdateSyntroStatusBox(SS_COMPONENT *)), Qt::DirectConnection);

	connect(&(m_server->m_multicastManager), SIGNAL(MMDisplay()), 
		m_multicastDlg, SLOT(MMDisplay()), Qt::QueuedConnection);
	connect(&(m_server->m_multicastManager), SIGNAL(MMRegistrationChanged(int)), 
		m_multicastDlg, SLOT(MMRegistrationChanged(int)), Qt::QueuedConnection);
	connect(&(m_server->m_multicastManager), SIGNAL(MMNewEntry(int)), 
		m_multicastDlg, SLOT(MMNewEntry(int)), Qt::QueuedConnection);
	connect(&(m_server->m_multicastManager), SIGNAL(MMDeleteEntry(int)), 
		m_multicastDlg, SLOT(MMDeleteEntry(int)), Qt::QueuedConnection);

	connect(&(m_server->m_dirManager), SIGNAL(DMNewDirectory(int)), 
		m_directoryDlg, SLOT(DMNewDirectory(int)), Qt::QueuedConnection);

	// This next connection has to be direct - ugly but necessary

	connect(&(m_server->m_dirManager), SIGNAL(DMRemoveDirectory(DM_COMPONENT *)), 
		m_directoryDlg, SLOT(DMRemoveDirectory(DM_COMPONENT *)), Qt::DirectConnection);

	m_server->resumeThread();

	// can't connect new hello emits to dialog yet so just start timer and do it when the time is right

	m_helloConnected = false;
	m_timer = startTimer(1000);

	connect(m_server, SIGNAL(serverMulticastUpdate(qint64, unsigned, qint64, unsigned)), 
		this, SLOT(serverMulticastUpdate(qint64, unsigned, qint64, unsigned)), Qt::QueuedConnection);
	connect(m_server, SIGNAL(serverE2EUpdate(qint64, unsigned, qint64, unsigned)), 
		this, SLOT(serverE2EUpdate(qint64, unsigned, qint64, unsigned)), Qt::QueuedConnection);

}

void SyntroControl::closeEvent(QCloseEvent *)
{
	killTimer(m_timer);
	m_server->exitThread();
	syntroAppExit();
	saveWindowState();
}

void SyntroControl::timerEvent(QTimerEvent *)
{
	if (!m_helloConnected && (m_server->getHelloObject() != NULL)) {
		connect(m_server->getHelloObject(), SIGNAL(helloDisplayEvent(Hello *)), m_helloDlg, SLOT(helloDisplayEvent(Hello *)), Qt::QueuedConnection); 
		m_helloConnected = true;
	}
}


void SyntroControl::onDirectory()
{
	m_directoryDlg->activateWindow();
	m_directoryDlg->show();
}

void SyntroControl::onHello()
{
	m_helloDlg->activateWindow();
	m_helloDlg->show();
}

void SyntroControl::onMulticast()
{
	m_multicastDlg->activateWindow();
	m_multicastDlg->show();
}

void SyntroControl::UpdateSyntroStatusBox(SS_COMPONENT *syntroComponent)
{
	HELLO *hello = &(syntroComponent->heartbeat.hello);

	if (syntroComponent->inUse && (syntroComponent->state == ConnNormal) && (hello != NULL)) {
		QString heartbeatInterval = QString ("%1") .arg(syntroComponent->heartbeatInterval / SYNTRO_CLOCKS_PER_SEC);
		ui.m_table->item(syntroComponent->index, 0)->setText(hello->componentType);
		ui.m_table->item(syntroComponent->index, 1)->setText(hello->componentName);
		ui.m_table->item(syntroComponent->index, 2)->setText(displayUID(&hello->componentUID));
		ui.m_table->item(syntroComponent->index, 3)->setText(displayIPAddr(hello->IPAddr));
		if (syntroComponent->tunnelSource)					// if I am a tunnel source...
            ui.m_table->item(syntroComponent->index, 4)->setText("Tunnel dest");	// ...indicate other end is a tunnel dest
		else
            ui.m_table->item(syntroComponent->index, 4)->setText(heartbeatInterval);
	}
	else {
		if (syntroComponent->tunnelStatic)
			(ui.m_table->item(syntroComponent->index, 0))->setText(syntroComponent->tunnelStaticName);
		else
			(ui.m_table->item(syntroComponent->index, 0))->setText("...");
		(ui.m_table->item(syntroComponent->index, 1))->setText("");
		(ui.m_table->item(syntroComponent->index, 2))->setText("");
		(ui.m_table->item(syntroComponent->index, 3))->setText("");
		(ui.m_table->item(syntroComponent->index, 4))->setText("");
	}
}

void SyntroControl::saveWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("ControlWindow");
		m_settings->setValue("Geometry", saveGeometry());
		m_settings->setValue("State", saveState());

		m_settings->beginWriteArray("Grid");
		for (int i = 0; i < ui.m_table->columnCount(); i++) {
			m_settings->setArrayIndex(i);
			m_settings->setValue("columnWidth", ui.m_table->columnWidth(i));
		}
		m_settings->endArray();

		m_settings->endGroup();
	}
}

void SyntroControl::restoreWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("ControlWindow");
		restoreGeometry(m_settings->value("Geometry").toByteArray());
		restoreState(m_settings->value("State").toByteArray());

		int count = m_settings->beginReadArray("Grid");
		for (int i = 0; i < count && i < ui.m_table->columnCount(); i++) {
			m_settings->setArrayIndex(i);
			int width = m_settings->value("columnWidth").toInt();

			if (width > 0)
				ui.m_table->setColumnWidth(i, width);
		}
		m_settings->endArray();

		m_settings->endGroup();
	}
}

void SyntroControl::onAbout()
{
	SyntroAbout *dlg = new SyntroAbout(this, m_settings);
	dlg->show();
}

void SyntroControl::onBasicSetup()
{
	ControlSetupDlg *dlg = new ControlSetupDlg(this, m_settings);
	dlg->show();
}


void SyntroControl::serverMulticastUpdate(qint64 in, unsigned inRate, qint64 out, unsigned outRate)
{
	m_serverMulticastStatus->setText(	QString("Mcast: in=") + QString::number(in) +
										QString(" out=") + QString::number(out) + 
										QString("  Mcast rate: in=") + QString::number(inRate) +
										QString(" out=") + QString::number(outRate));
}


void SyntroControl::serverE2EUpdate(qint64 in, unsigned inRate, qint64 out, unsigned outRate)
{
	m_serverE2EStatus->setText(			QString("E2E: in=") + QString::number(in) +
										QString(" out=") + QString::number(out) + 
										QString("  E2E rate: in=") + QString::number(inRate) +
										QString(" out=") + QString::number(outRate));
}
