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

#include "SyntroExec.h"
#include "SyntroAboutDlg.h"
#include "ConfigureDlg.h"

#define DEFAULT_ROW_HEIGHT 20


SyntroExec::SyntroExec(QSettings *settings, QWidget *parent)
	: QMainWindow(parent), m_settings(settings)
{
	m_suppressSignals = true;
	ui.setupUi(this);

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionHello, SIGNAL(triggered()), this, SLOT(onHello()));
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));

	layoutTable();
 
	restoreWindowState();

	m_helloDlg = new HelloDialog(this);
	m_helloDlg->setModal(false);

	setWindowTitle(QString("%1 - %2")
		.arg(m_settings->value(SYNTRO_PARAMS_APPNAME).toString())
		.arg(m_settings->value(SYNTRO_PARAMS_COMPTYPE).toString()));

	SyntroUtils::syntroAppInit(m_settings);

	m_manager = new ComponentManager(settings);
	connect(m_manager, SIGNAL(updateExecStatus(ManagerComponent *)), this, SLOT(updateExecStatus(ManagerComponent *)), Qt::DirectConnection);
	connect(this, SIGNAL(loadComponent(int)), m_manager, SLOT(loadComponent(int)), Qt::QueuedConnection);
	m_manager->resumeThread();

	for (int i = 0; i < SYNTRO_MAX_COMPONENTSPERDEVICE; i++)
		emit loadComponent(i);

	connect(m_manager->getHelloObject(), SIGNAL(helloDisplayEvent(Hello *)), m_helloDlg, SLOT(helloDisplayEvent(Hello *)), Qt::QueuedConnection); 

	m_suppressSignals = false;
}

void SyntroExec::layoutTable()
{
	QTableWidgetItem *item;

	int cellHeight = fontMetrics().lineSpacing() + 6;
	
	m_table = new QTableWidget(this);

	m_table->setColumnCount(SYNTROEXEC_COL_COUNT);
	m_table->setColumnWidth(SYNTROEXEC_COL_CONFIG, 80);
	m_table->setColumnWidth(SYNTROEXEC_COL_INUSE, 60);
	m_table->setColumnWidth(SYNTROEXEC_COL_APPNAME, 140);
	m_table->setColumnWidth(SYNTROEXEC_COL_COMPNAME, 140);
	m_table->setColumnWidth(SYNTROEXEC_COL_UID, 140);
	m_table->setColumnWidth(SYNTROEXEC_COL_STATE, 140);
	m_table->setColumnWidth(SYNTROEXEC_COL_HELLOSTATE, 100);

	m_table->verticalHeader()->setDefaultSectionSize(cellHeight);

    m_table->setHorizontalHeaderLabels(QStringList() << tr("") << tr("In use") << tr("App name") 
			<< tr("Component name") << tr("UID") 
			<< tr("Execution state") << tr("Hello state"));
  
    setMinimumSize(620, 120);

    m_table->setSelectionMode(QAbstractItemView::NoSelection);
 
	for (int row = 0; row < SYNTRO_MAX_COMPONENTSPERDEVICE; row++) {
		m_table->insertRow(row);
		m_table->setRowHeight(row, cellHeight);
		m_table->setContentsMargins(5, 5, 5, 5);

		ExecButton *button = new ExecButton("Configure", this, row);
		m_table->setCellWidget(row, SYNTROEXEC_COL_CONFIG, button);
		if (row == INSTANCE_EXEC)
			button->setEnabled(false);
		else
			connect(button, SIGNAL(buttonClicked(int)), this, SLOT(buttonClicked(int)));

		m_useBox[row] = new ExecCheckBox(m_table, row);
		m_table->setCellWidget(row, SYNTROEXEC_COL_INUSE, m_useBox[row]);
		connect(m_useBox[row], SIGNAL(boxClicked(bool, int)), this, SLOT(boxClicked(bool, int)));
		
		for (int col = 2; col < SYNTROEXEC_COL_COUNT; col++) {
			item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignLeft | Qt::AlignBottom);
			item->setFlags(Qt::ItemIsEnabled);
			item->setText("");
			m_table->setItem(row, col, item);
		}
	}
	setCentralWidget(m_table);
}


void SyntroExec::closeEvent(QCloseEvent *)
{
	m_manager->exitThread();
	saveWindowState();
	SyntroUtils::syntroAppExit();
}


void SyntroExec::onHello()
{
	m_helloDlg->activateWindow();
	m_helloDlg->show();
}

void SyntroExec:: updateExecStatus(ManagerComponent *component)
{
	QMutexLocker locker(&(m_manager->m_lock));

	m_suppressSignals = true;

	m_table->item(component->instance, SYNTROEXEC_COL_APPNAME)->setText(component->appName);
	m_table->item(component->instance, SYNTROEXEC_COL_STATE)->setText(component->processState);
	m_useBox[component->instance]->setCheckState(component->inUse ? Qt::Checked : Qt::Unchecked);

	if (component->inUse) {
		m_table->item(component->instance, SYNTROEXEC_COL_UID)->setText(SyntroUtils::displayUID(&component->UID));
		if (component->monitored) {
			m_table->item(component->instance, SYNTROEXEC_COL_HELLOSTATE)->setText(component->helloStateString);
			m_table->item(component->instance, SYNTROEXEC_COL_COMPNAME)->setText(component->componentName);
		} else {
			m_table->item(component->instance, SYNTROEXEC_COL_HELLOSTATE)->setText("...");
			m_table->item(component->instance, SYNTROEXEC_COL_COMPNAME)->setText("...");
		}
	} else {
		m_table->item(component->instance, SYNTROEXEC_COL_UID)->setText("");
		m_table->item(component->instance, SYNTROEXEC_COL_HELLOSTATE)->setText("");
		m_table->item(component->instance, SYNTROEXEC_COL_COMPNAME)->setText("");
	}


	m_suppressSignals = false;
}

void SyntroExec::saveWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("ControlWindow");
		m_settings->setValue("Geometry", saveGeometry());
		m_settings->setValue("State", saveState());

		m_settings->beginWriteArray("Grid");
		for (int i = 0; i < m_table->columnCount(); i++) {
			m_settings->setArrayIndex(i);
			m_settings->setValue("columnWidth", m_table->columnWidth(i));
		}
		m_settings->endArray();
		m_settings->endGroup();
	}
}

void SyntroExec::restoreWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("ControlWindow");
		restoreGeometry(m_settings->value("Geometry").toByteArray());
		restoreState(m_settings->value("State").toByteArray());

		int count = m_settings->beginReadArray("Grid");
		for (int i = 0; i < count && i < m_table->columnCount(); i++) {
			m_settings->setArrayIndex(i);
			int width = m_settings->value("columnWidth").toInt();

			if (width > 0)
				m_table->setColumnWidth(i, width);
		}
		m_settings->endArray();
		m_settings->endGroup();
	}
}

void SyntroExec::onAbout()
{
	SyntroAbout *dlg = new SyntroAbout(this, m_settings);
	dlg->show();
}

void SyntroExec::buttonClicked(int buttonId)
{
	if (m_suppressSignals)
		return;												// avoids thrashing when loading config

	if (buttonId == INSTANCE_EXEC)
		return;												// cannot configure Exec
	ConfigureDlg *dlg = new ConfigureDlg(this, m_settings, buttonId);
	if (dlg->exec()) {										// need to update client with changes
		emit loadComponent(buttonId);
	}
}

void SyntroExec::boxClicked(bool, int boxId)
{
	if (m_suppressSignals)
		return;												// avoids thrashing when loading config
	m_settings->beginWriteArray(SYNTROEXEC_PARAMS_COMPONENTS);
	m_settings->setArrayIndex(boxId);

	if (m_useBox[boxId]->checkState() == Qt::Checked)
		m_settings->setValue(SYNTROEXEC_PARAMS_INUSE, SYNTRO_PARAMS_TRUE);
	else
		m_settings->setValue(SYNTROEXEC_PARAMS_INUSE, SYNTRO_PARAMS_FALSE);
	m_settings->endArray();
	emit loadComponent(boxId);
}



//----------------------------------------------------------
//
// ExecButton functions

ExecButton::ExecButton(const QString& text, QWidget *parent, int id)
	: QPushButton(text, parent) 
{
	m_id = id;
	connect(this, SIGNAL(clicked(bool)), this, SLOT(originalClicked(bool)));
};

void ExecButton::originalClicked(bool )
{
	emit buttonClicked(m_id);
}

//----------------------------------------------------------
//
// ExecCheckBox functions

ExecCheckBox::ExecCheckBox(QWidget *parent, int id)
	: QCheckBox(parent) 
{
	m_id = id;
	connect(this, SIGNAL(clicked(bool)), this, SLOT(originalClicked(bool)));
};

void ExecCheckBox::originalClicked(bool state)
{
	emit boxClicked(state, m_id);
}
