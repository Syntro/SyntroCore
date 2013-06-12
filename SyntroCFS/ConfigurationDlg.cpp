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

#include "ConfigurationDlg.h"
#include "SyntroCFS.h"


ConfigurationDlg::ConfigurationDlg(QWidget *parent, QSettings *settings)
	: QDialog(parent, Qt::WindowCloseButtonHint | Qt::WindowTitleHint)
{
	m_settings = settings;
	layoutWidgets();
	connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonClick()));
	connect(m_okButton, SIGNAL(clicked()), this, SLOT(okButtonClick()));
	loadCurrentValues();
}


void ConfigurationDlg::okButtonClick()
{
	if ((m_settings->value(SYNTRO_CFS_STORE_PATH).toString() != m_storePath->text()) ||
		(m_settings->value(SYNTRO_CFS_SERVICE_NAME).toString() != m_serviceName->text())) {

		m_settings->setValue(SYNTRO_CFS_SERVICE_NAME, m_serviceName->text());
		m_settings->setValue(SYNTRO_CFS_STORE_PATH, m_storePath->text());
		accept();
		return;
	}
	reject();
}

void ConfigurationDlg::cancelButtonClick()
{
	reject();
}

void ConfigurationDlg::loadCurrentValues()
{
	m_serviceName->setText(m_settings->value(SYNTRO_CFS_SERVICE_NAME).toString());
	m_storePath->setText(m_settings->value(SYNTRO_CFS_STORE_PATH).toString());
}

void ConfigurationDlg::layoutWidgets()
{
	QFormLayout *formLayout = new QFormLayout;

	m_serviceName = new QLineEdit;
	m_serviceName->setMinimumWidth(200);
	m_serviceName->setValidator(&m_validator);
	formLayout->addRow(new QLabel("Service name"), m_serviceName);

	m_storePath = new CFSLabel("");
	m_storePath->setMinimumWidth(320);
	formLayout->addRow(new QLabel("Store path"), m_storePath);

	QHBoxLayout *buttonLayout = new QHBoxLayout;

	m_okButton = new QPushButton("Ok");
	m_cancelButton = new QPushButton("Cancel");

	buttonLayout->addStretch();
	buttonLayout->addWidget(m_okButton);
	buttonLayout->addWidget(m_cancelButton);
	buttonLayout->addStretch();

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(formLayout);
	mainLayout->addSpacing(20);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);

	setWindowTitle("Configure SyntroCFS");
}


//----------------------------------------------------------
//
//	CFSLabel functions

CFSLabel::CFSLabel(const QString& text) : QLabel(text)
{
	setFrameStyle(QFrame::Sunken | QFrame::Panel);
}

void CFSLabel::mousePressEvent (QMouseEvent *)
{
	QFileDialog *fileDialog;

	fileDialog = new QFileDialog(this, "Store folder path");
	fileDialog->setFileMode(QFileDialog::DirectoryOnly);
	fileDialog->selectFile(text());
	if (fileDialog->exec())
		setText(fileDialog->selectedFiles().at(0));
}
