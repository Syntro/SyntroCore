//
//  Copyright (c) 2012, 2013 Pansenti, LLC.
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

#include "ControlSetupDlg.h"
#include "SyntroControl.h"

ControlSetupDlg::ControlSetupDlg()
	: QDialog()
{
	layoutWindow();
	setWindowTitle("Basic setup");
	connect(m_buttons, SIGNAL(accepted()), this, SLOT(onOk()));
    connect(m_buttons, SIGNAL(rejected()), this, SLOT(onCancel()));
}

ControlSetupDlg::~ControlSetupDlg()
{
	delete m_validator;
}

void ControlSetupDlg::onOk()
{
	QMessageBox msgBox;
	bool changed;

	changed = false;

	// check to see if any setting has changed

	QSettings *settings = SyntroUtils::getSettings();

	changed = m_appName->text() != settings->value(SYNTRO_PARAMS_APPNAME).toString();
	changed |= m_priority->text() != settings->value(SYNTRO_PARAMS_LOCALCONTROL_PRI).toString();

	if (!changed)
		reject();
	else {
		msgBox.setText("The component must be restarted for these changes to take effect");
		msgBox.exec();

		// save changes to settings

		settings->setValue(SYNTRO_PARAMS_APPNAME, m_appName->text());
		settings->setValue(SYNTRO_PARAMS_LOCALCONTROL_PRI, m_priority->text());
		
		delete settings;

		accept();
	}
}

void ControlSetupDlg::onCancel()
{
	reject();
}

void ControlSetupDlg::layoutWindow()
{
    setModal(true);

	QSettings *settings = SyntroUtils::getSettings();

	QVBoxLayout *centralLayout = new QVBoxLayout(this);
	centralLayout->setSpacing(20);
	centralLayout->setContentsMargins(11, 11, 11, 11);
	
	QFormLayout *formLayout = new QFormLayout();
	formLayout->setSpacing(16);
	formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

	m_appName = new QLineEdit(this);
	m_appName->setMinimumWidth(200);
	formLayout->addRow(tr("App name:"), m_appName);
	m_appName->setText(settings->value(SYNTRO_PARAMS_APPNAME).toString());

	m_validator = new ServiceNameValidator();
	m_appName->setValidator(m_validator);

	centralLayout->addLayout(formLayout);

	m_priority = new QLineEdit();
	formLayout->addRow(tr("Priority (0 (lowest) - 255):"), m_priority);
	m_priority->setText(settings->value(SYNTRO_PARAMS_LOCALCONTROL_PRI).toString());
	m_priority->setValidator(new QIntValidator(0, 255));

	m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	m_buttons->setCenterButtons(true);

	centralLayout->addWidget(m_buttons);
}

