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

#include <qfiledialog.h>
#include <qmessagebox.h>

#include "SyntroDefs.h"
#include "SyntroLog.h"
#include "SyntroAboutDlg.h"
#include "BasicSetupDlg.h"

SyntroLog::SyntroLog(QSettings *settings, QWidget *parent)
	: QMainWindow(parent), m_settings(settings)
{
	ui.setupUi(this);
	initStatusBar();
	initGrid();

	m_activeClientCount = 0;

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(onSave()));
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
	connect(ui.actionBasicSetup, SIGNAL(triggered()), this, SLOT(onBasicSetup()));

	SyntroUtils::syntroAppInit(m_settings);

	m_client = new LogClient(this, settings);
	connect(m_client, SIGNAL(newLogMsg(QByteArray)), this, SLOT(newLogMsg(QByteArray)), Qt::DirectConnection);
	connect(m_client, SIGNAL(activeClientUpdate(int)), this, SLOT(activeClientUpdate(int)), Qt::DirectConnection);
	m_client->resumeThread();

	restoreWindowState();

	setWindowTitle(QString("%1 - %2")
		.arg(m_settings->value(SYNTRO_PARAMS_APPNAME).toString())
		.arg(m_settings->value(SYNTRO_PARAMS_COMPTYPE).toString()));

	m_timer = startTimer(500);
}

void SyntroLog::closeEvent(QCloseEvent *)
{
	if (m_timer) {
		killTimer(m_timer);
		m_timer = 0;
	}

	if (m_client) {
		disconnect(m_client, SIGNAL(newLogMsg(QByteArray)), this, SLOT(newLogMsg(QByteArray)));
		disconnect(m_client, SIGNAL(activeClientUpdate(int)), this, SLOT(activeClientUpdate(int)));
		m_client->exitThread();
		m_client->wait();
	}

	saveWindowState();
	SyntroUtils::syntroAppExit();
}

void SyntroLog::newLogMsg(QByteArray bulkMsg)
{
	QMutexLocker lock(&m_logMutex);
	m_logQ.enqueue(bulkMsg);
}

void SyntroLog::activeClientUpdate(int count)
{
	QMutexLocker lock(&m_activeClientMutex);
	m_activeClientCount = count;
}

void SyntroLog::timerEvent(QTimerEvent *)
{
	m_controlStatus->setText(m_client->getLinkState());

	m_activeClientMutex.lock();
	m_activeClientStatus->setText(QString("Active clients - %1").arg(m_activeClientCount));
	m_activeClientMutex.unlock();

	parseMsgQueue();
}

void SyntroLog::parseMsgQueue()
{
	QMutexLocker lock(&m_logMutex);

	while (!m_logQ.empty()) {
		QByteArray bulkMsg = m_logQ.dequeue();
		QString s(bulkMsg);
		QStringList list = s.split('\n');

		for (int i = 0; i < list.count(); i++)
			addMessage(list[i]);
	}
}

void SyntroLog::addMessage(QString msg)
{
	QStringList field = msg.split(SYNTRO_LOG_COMPONENT_SEP);

	if (field.count() != ui.logTable->columnCount())
		return;

	field[1].replace('T', ' ');

	int row = findRowInsertPosition(field[1]);

	ui.logTable->insertRow(row);

	// log level
	QTableWidgetItem *item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setText(field[0]);

	if (field[0] == "ERROR")
		item->setBackgroundColor(Qt::red);
	else if (field[0] == "WARN")
		item->setBackgroundColor(Qt::yellow);
	else if (field[0] == "DEBUG")
		item->setBackgroundColor(Qt::green);
	else if (field[0] == "LOCAL")
		item->setBackgroundColor(Qt::gray);


	ui.logTable->setItem(row, 0, item);


	// timestamp
	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setText(field[1]);
	ui.logTable->setItem(row, 1, item);


	// UID
	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setText(field[2]);
	ui.logTable->setItem(row, 2, item);


	// client name
	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);

	if (field[3].endsWith(SYNTRO_LOG_SERVICE_TAG))
		item->setText(field[3].left(field[3].length() - strlen(SYNTRO_LOG_SERVICE_TAG)));
	else
		item->setText(field[3]);

	ui.logTable->setItem(row, 3, item);
	

	// the log message
	item = new QTableWidgetItem();
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setText(field[4]);
	ui.logTable->setItem(row, 4, item);

	ui.logTable->scrollToBottom();
}

int SyntroLog::findRowInsertPosition(QString timestamp)
{
	for (int i = ui.logTable->rowCount() - 1; i >= 0; i--) {
		QTableWidgetItem *item = ui.logTable->item(i, 1);

		if (timestamp >= item->text())
			return i + 1;
	}

	return 0;
}

void SyntroLog::onSave()
{
	QFile file;
	QString fileName = QFileDialog::getSaveFileName(this, "SyntroLog Save", m_savePath, QString("Logs (*.log)"));

	if (fileName.isEmpty())
		return;

	file.setFileName(fileName);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QMessageBox::warning(this, "SyntroLog Save", "File save failed");
		return;
	}

	QTextStream textStream(&file);

	m_logMutex.lock();

	for (int i = 0; i < ui.logTable->rowCount(); i++) {
		for (int j = 0; j < ui.logTable->columnCount(); j++) {
			textStream << ui.logTable->item(i, j)->text();

			if (j == ui.logTable->columnCount() - 1)
				textStream << endl;
			else
				textStream << SYNTRO_LOG_COMPONENT_SEP;
		}
	}

	m_logMutex.unlock();

	// for next time
	m_savePath = fileName;
}

void SyntroLog::initGrid()
{
	ui.logTable->setColumnCount(5);
	ui.logTable->horizontalHeader()->setStretchLastSection(true);

	ui.logTable->verticalHeader()->setDefaultSectionSize(fontMetrics().height() + 4);
	ui.logTable->setSelectionMode(QAbstractItemView::ContiguousSelection);
	ui.logTable->setSelectionBehavior(QAbstractItemView::SelectRows);

	ui.logTable->setHorizontalHeaderLabels(QStringList() << "Level" << "Timestamp" << "UID" << "Component" << "Message");
}

void SyntroLog::initStatusBar()
{
	m_controlStatus = new QLabel(this);
	m_controlStatus->setAlignment(Qt::AlignLeft);
	ui.statusBar->addWidget(m_controlStatus, 1);

	m_activeClientStatus = new QLabel(this);
	m_activeClientStatus->setAlignment(Qt::AlignRight);
	ui.statusBar->addWidget(m_activeClientStatus);
}

void SyntroLog::onAbout()
{
	SyntroAbout *dlg = new SyntroAbout(this, m_settings);
	dlg->show();
}

void SyntroLog::onBasicSetup()
{
	BasicSetupDlg *dlg = new BasicSetupDlg(this, m_settings);
	dlg->show();
}

void SyntroLog::saveWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("Window");
		m_settings->setValue("Geometry", saveGeometry());
		m_settings->setValue("State", saveState());

		m_settings->beginWriteArray("Grid");
		for (int i = 0; i < ui.logTable->columnCount() - 1; i++) {
			m_settings->setArrayIndex(i);
			m_settings->setValue("columnWidth", ui.logTable->columnWidth(i));
		}
		m_settings->endArray();

		m_settings->endGroup();
	}
}

void SyntroLog::restoreWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("Window");
		restoreGeometry(m_settings->value("Geometry").toByteArray());
		restoreState(m_settings->value("State").toByteArray());

		int count = m_settings->beginReadArray("Grid");
		for (int i = 0; i < count && i < ui.logTable->columnCount() - 1; i++) {
			m_settings->setArrayIndex(i);
			int width = m_settings->value("columnWidth").toInt();

			if (width > 0)
				ui.logTable->setColumnWidth(i, width);
		}
		m_settings->endArray();

		m_settings->endGroup();
	}
}
