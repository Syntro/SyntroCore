//
//  Copyright (c) 2012 Pansenti, LLC.
//	
//  This file is part of SyntroLib
//
//  SyntroLib is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  SyntroLib is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with SyntroLib.  If not, see <http://www.gnu.org/licenses/>.
//

#include "BasicSetupDlg.h"
#include "SyntroUtils.h"
#include <qboxlayout.h>
#include <qformlayout.h>

BasicSetupDlg::BasicSetupDlg(QWidget *parent, QSettings *settings)
	: QDialog(parent), m_settings(settings)
{
	layoutWindow();
	setWindowTitle("Basic setup");
	connect(m_buttons, SIGNAL(accepted()), this, SLOT(onOk()));
    connect(m_buttons, SIGNAL(rejected()), this, SLOT(onCancel()));
}

BasicSetupDlg::~BasicSetupDlg()
{
	delete m_validator;
}

void BasicSetupDlg::onOk()
{
	QMessageBox msgBox;

	// check to see if any setting has changed

	if (m_componentName->text() != m_settings->value(SYNTRO_PARAMS_APPNAME).toString())
		goto goChanged;

	m_settings->beginReadArray(SYNTRO_PARAMS_CONTROL_NAMES);
	for (int control = 0; control < ENDPOINT_MAX_SYNTROCONTROLS; control++) {
		m_settings->setArrayIndex(control);
		if (m_controlName[control]->text() != m_settings->value(SYNTRO_PARAMS_CONTROL_NAME).toString()) {
			m_settings->endArray();
			goto goChanged;
		}
	}
	m_settings->endArray();

	if ((m_revert->checkState() == Qt::Checked) != m_settings->value(SYNTRO_PARAMS_CONTROLREVERT).toBool())
		goto goChanged;

	if ((m_localControl->checkState() == Qt::Checked) != m_settings->value(SYNTRO_PARAMS_LOCALCONTROL).toBool())
		goto goChanged;

	if (m_localControlPri->text() !=  m_settings->value(SYNTRO_PARAMS_LOCALCONTROL_PRI).toString())
		goto goChanged;

	if (m_adaptor->currentText() == "<any>") {
		if (m_settings->value(SYNTRO_RUNTIME_ADAPTER).toString() != "")
			goto goChanged;
	} else {
		if (m_adaptor->currentText() != m_settings->value(SYNTRO_RUNTIME_ADAPTER).toString())
			goto goChanged;
	}
	
	reject();
	return;

goChanged:
	// save changes to settings

	m_settings->setValue(SYNTRO_PARAMS_APPNAME, m_componentName->text());

	m_settings->beginWriteArray(SYNTRO_PARAMS_CONTROL_NAMES);
	for (int control = 0; control < ENDPOINT_MAX_SYNTROCONTROLS; control++) {
		m_settings->setArrayIndex(control);
		m_settings->setValue(SYNTRO_PARAMS_CONTROL_NAME, m_controlName[control]->text());
	}
	m_settings->endArray();

	m_settings->setValue(SYNTRO_PARAMS_CONTROLREVERT, m_revert->checkState() == Qt::Checked);
	m_settings->setValue(SYNTRO_PARAMS_LOCALCONTROL, m_localControl->checkState() == Qt::Checked);
	m_settings->setValue(SYNTRO_PARAMS_LOCALCONTROL_PRI, m_localControlPri->text());

	if (m_adaptor->currentText() == "<any>")
		m_settings->setValue(SYNTRO_RUNTIME_ADAPTER, "");
	else
		m_settings->setValue(SYNTRO_RUNTIME_ADAPTER, m_adaptor->currentText());

	msgBox.setText("The component must be restarted for these changes to take effect");
	msgBox.setIcon(QMessageBox::Warning);
	msgBox.exec();

	accept();
}

void BasicSetupDlg::onCancel()
{
	reject();
}

void BasicSetupDlg::layoutWindow()
{
    setModal(true);

	QVBoxLayout *centralLayout = new QVBoxLayout(this);
	centralLayout->setSpacing(20);
	centralLayout->setContentsMargins(11, 11, 11, 11);
	
	QFormLayout *formLayout = new QFormLayout();
	formLayout->setSpacing(16);
	formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

	m_componentName = new QLineEdit(this);
	m_componentName->setToolTip("The name of this instance of the app.");
	m_componentName->setMinimumWidth(200);
	formLayout->addRow(tr("Component name:"), m_componentName);
	m_validator = new ServiceNameValidator();
	m_componentName->setValidator(m_validator);
	m_componentName->setText(m_settings->value(SYNTRO_PARAMS_APPNAME).toString());

	m_settings->beginReadArray(SYNTRO_PARAMS_CONTROL_NAMES);
	for (int control = 0; control < ENDPOINT_MAX_SYNTROCONTROLS; control++) {
		m_settings->setArrayIndex(control);
		m_controlName[control] = new QLineEdit(this);
		m_controlName[control]->setToolTip("The component name of the SyntroControl to which the app should connect.\n If empty, use any available SyntroControl.");
		m_controlName[control]->setMinimumWidth(200);
		formLayout->addRow(tr("SyntroControl (priority %1):").arg(ENDPOINT_MAX_SYNTROCONTROLS - control), m_controlName[control]);
		m_controlName[control]->setText(m_settings->value(SYNTRO_PARAMS_CONTROL_NAME).toString());
		m_controlName[control]->setValidator(m_validator);
	}
	m_settings->endArray();

	m_revert = new QCheckBox();
	formLayout->addRow(tr("Revert to best SyntroControl:"), m_revert);
	if (m_settings->value(SYNTRO_PARAMS_CONTROLREVERT).toBool())
		m_revert->setCheckState(Qt::Checked);

	m_localControl = new QCheckBox();
	formLayout->addRow(tr("Enable internal SyntroControl:"), m_localControl);
	if (m_settings->value(SYNTRO_PARAMS_LOCALCONTROL).toBool())
		m_localControl->setCheckState(Qt::Checked);

	m_localControlPri = new QLineEdit();
	formLayout->addRow(tr("Internal SyntroControl priority (0 (lowest) - 255):"), m_localControlPri);
	m_localControlPri->setText(m_settings->value(SYNTRO_PARAMS_LOCALCONTROL_PRI).toString());
	m_localControlPri->setValidator(new QIntValidator(0, 255));

	m_adaptor = new QComboBox;
	m_adaptor->setEditable(false);
	populateAdaptors();
	QHBoxLayout *a = new QHBoxLayout;
	a->addWidget(m_adaptor);
	a->addStretch();
	formLayout->addRow(new QLabel("Ethernet adaptor:"), a);
	int findIndex = m_adaptor->findText(m_settings->value(SYNTRO_RUNTIME_ADAPTER).toString());
	if (findIndex != -1)
		m_adaptor->setCurrentIndex(findIndex);
	else
		m_adaptor->setCurrentIndex(0);

	centralLayout->addLayout(formLayout);

	m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	m_buttons->setCenterButtons(true);

	centralLayout->addWidget(m_buttons);
}

void BasicSetupDlg::populateAdaptors()
{
	QNetworkInterface		cInterface;
	int index;

	m_adaptor->insertItem(0, "<any>");
	index = 1;
	QList<QNetworkInterface> ani = QNetworkInterface::allInterfaces();
	foreach (cInterface, ani)
		m_adaptor->insertItem(index++, cInterface.humanReadableName());
}
