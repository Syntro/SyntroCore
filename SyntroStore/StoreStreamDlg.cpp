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

#include "StoreStreamDlg.h"
#include "StoreStream.h"
#include "SyntroStore.h"


StoreStreamDlg::StoreStreamDlg(QWidget *parent, QSettings *settings, int index)
	: QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
	m_index = index;
	m_settings = settings;
	layoutWidgets();
	connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonClick()));
	connect(m_okButton, SIGNAL(clicked()), this, SLOT(okButtonClick()));
	loadCurrentValues();
}


void StoreStreamDlg::okButtonClick()
{
	m_settings->beginWriteArray(SYNTROSTORE_PARAMS_STREAM_SOURCES);
	m_settings->setArrayIndex(m_index);

	if (m_formatCombo->currentIndex() == 0)
		m_settings->setValue(SYNTROSTORE_PARAMS_FORMAT, SYNTRO_RECORD_STORE_FORMAT_SRF);
	else
		m_settings->setValue(SYNTROSTORE_PARAMS_FORMAT, SYNTRO_RECORD_STORE_FORMAT_RAW);

	m_settings->setValue(SYNTROSTORE_PARAMS_PATH, m_storePath->text());
	m_settings->setValue(SYNTROSTORE_PARAMS_CREATE_SUBFOLDER, 
		m_subFolderCheck->checkState() == Qt::Checked ? SYNTRO_PARAMS_TRUE : SYNTRO_PARAMS_FALSE);

	switch (m_rotationPolicy->currentIndex()) {
		case 0:
			m_settings->setValue(SYNTROSTORE_PARAMS_ROTATION_POLICY, SYNTROSTORE_PARAMS_ROTATION_POLICY_TIME);
			break;

		case 1:
			m_settings->setValue(SYNTROSTORE_PARAMS_ROTATION_POLICY, SYNTROSTORE_PARAMS_ROTATION_POLICY_SIZE);
			break;

		default:
			m_settings->setValue(SYNTROSTORE_PARAMS_ROTATION_POLICY, SYNTROSTORE_PARAMS_ROTATION_POLICY_ANY);
			break;
	}

	if (m_rotationTimeUnits->currentIndex() == 1)
		m_settings->setValue(SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS, SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS_HOURS);
	else	
		m_settings->setValue(SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS, SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS_MINUTES);
		
	m_settings->setValue(SYNTROSTORE_PARAMS_STREAM_SOURCE, m_streamName->text());
	
	m_settings->setValue(SYNTROSTORE_PARAMS_ROTATION_TIME, m_rotationTime->text());
	m_settings->setValue(SYNTROSTORE_PARAMS_ROTATION_SIZE, m_rotationSize->text());

	m_settings->endArray();
	accept();
}

void StoreStreamDlg::cancelButtonClick()
{
	reject();
}



void StoreStreamDlg::loadCurrentValues()
{
	m_settings->beginReadArray(SYNTROSTORE_PARAMS_STREAM_SOURCES);
	m_settings->setArrayIndex(m_index);

	m_streamName->setText(m_settings->value(SYNTROSTORE_PARAMS_STREAM_SOURCE).toString());

	if (m_settings->value(SYNTROSTORE_PARAMS_FORMAT) == SYNTRO_RECORD_STORE_FORMAT_SRF)
		m_formatCombo->setCurrentIndex(0);
	else
		m_formatCombo->setCurrentIndex(1);

	m_storePath->setText(m_settings->value(SYNTROSTORE_PARAMS_PATH).toString());

	if (m_settings->value(SYNTROSTORE_PARAMS_CREATE_SUBFOLDER) == SYNTRO_PARAMS_TRUE)
		m_subFolderCheck->setCheckState(Qt::Checked);
	else
		m_subFolderCheck->setCheckState(Qt::Unchecked);

	QString rotate = m_settings->value(SYNTROSTORE_PARAMS_ROTATION_POLICY).toString();
	if (rotate == SYNTROSTORE_PARAMS_ROTATION_POLICY_TIME)
		m_rotationPolicy->setCurrentIndex(0);
	else if (rotate == SYNTROSTORE_PARAMS_ROTATION_POLICY_SIZE)
		m_rotationPolicy->setCurrentIndex(1);
	else
		m_rotationPolicy->setCurrentIndex(2);

	if (m_settings->value(SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS).toString() == SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS_HOURS)
		m_rotationTimeUnits->setCurrentIndex(1);
	else
		m_rotationTimeUnits->setCurrentIndex(0);

	m_rotationTime->setText(m_settings->value(SYNTROSTORE_PARAMS_ROTATION_TIME).toString());
	m_rotationSize->setText(m_settings->value(SYNTROSTORE_PARAMS_ROTATION_SIZE).toString());

	m_settings->endArray();
}

void StoreStreamDlg::layoutWidgets()
{
	QFormLayout *formLayout = new QFormLayout;

	QHBoxLayout *e = new QHBoxLayout;
	e->addStretch();
	formLayout->addRow(new QLabel("Store stream"), e);

	m_formatCombo = new QComboBox;
	m_formatCombo->addItem("Structured [srf])");
	m_formatCombo->addItem("Raw [raw]");
	m_formatCombo->setEditable(false);
	QHBoxLayout *a = new QHBoxLayout;
	a->addWidget(m_formatCombo);
	a->addStretch();
	formLayout->addRow(new QLabel("Store format"), a);

	m_streamName = new QLineEdit;
	m_streamName->setMinimumWidth(200);
	m_streamName->setValidator(&m_validator);
	formLayout->addRow(new QLabel("Stream path"), m_streamName);

	m_storePath = new StoreLabel("");
	m_storePath->setMinimumWidth(320);
	formLayout->addRow(new QLabel("Store path"), m_storePath);

	m_subFolderCheck = new QCheckBox;
	formLayout->addRow(new QLabel("Create subfolder"), m_subFolderCheck);

	m_rotationPolicy = new QComboBox;
	m_rotationPolicy->addItem(SYNTROSTORE_PARAMS_ROTATION_POLICY_TIME);
	m_rotationPolicy->addItem(SYNTROSTORE_PARAMS_ROTATION_POLICY_SIZE);
	m_rotationPolicy->addItem(SYNTROSTORE_PARAMS_ROTATION_POLICY_ANY);
	m_rotationPolicy->setEditable(false);
	QHBoxLayout *b = new QHBoxLayout;
	b->addWidget(m_rotationPolicy);
	b->addStretch();
	formLayout->addRow(new QLabel("Rotation policy"), b);

	m_rotationTimeUnits = new QComboBox;
	m_rotationTimeUnits->addItem(SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS_MINUTES);
	m_rotationTimeUnits->addItem(SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS_HOURS);
	m_rotationTimeUnits->setEditable(false);
	QHBoxLayout *c = new QHBoxLayout;
	c->addWidget(m_rotationTimeUnits);
	c->addStretch();
	formLayout->addRow(new QLabel("Rotation time units"), c);
		
	m_rotationTime = new QLineEdit;
	m_rotationTime->setMaximumWidth(100);
	formLayout->addRow(new QLabel("Rotation time"), m_rotationTime);

	m_rotationSize = new QLineEdit;
	m_rotationSize->setMaximumWidth(100);
	formLayout->addRow(new QLabel("Rotation size (MB)"), m_rotationSize);

	QHBoxLayout *buttonLayout = new QHBoxLayout;

	m_okButton = new QPushButton("Ok");
	m_cancelButton = new QPushButton("Cancel");

	buttonLayout->addStretch();
	buttonLayout->addWidget(m_okButton);
	buttonLayout->addWidget(m_cancelButton);
	buttonLayout->addStretch();

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(formLayout);
	mainLayout->addSpacing(20);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);

	setWindowTitle("Configure entry");
}

//----------------------------------------------------------
//
//	StoreLabel functions

StoreLabel::StoreLabel(const QString& text) : QLabel(text)
{
	setFrameStyle(QFrame::Sunken | QFrame::Panel);
}

void StoreLabel::mousePressEvent (QMouseEvent *)
{
	QFileDialog *fileDialog;

	fileDialog = new QFileDialog(this, "Store folder path");
	fileDialog->setFileMode(QFileDialog::DirectoryOnly);
	fileDialog->selectFile(text());
	if (fileDialog->exec())
		setText(fileDialog->selectedFiles().at(0));
}
