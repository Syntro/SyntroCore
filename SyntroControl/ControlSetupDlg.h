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

#ifndef CONTROLSETUPDLG_H
#define CONTROLSETUPDLG_H

#include "SyntroLib.h"
#include "SyntroValidators.h"
#include <qdialog.h>
#include <qlineedit.h>
#include <qdialogbuttonbox.h>
#include <qmessagebox.h>
#include <qformlayout.h>

class ControlSetupDlg : public QDialog
{
	Q_OBJECT

public:
	ControlSetupDlg();
	~ControlSetupDlg();

public slots:
	void onOk();
	void onCancel();

private:
	void layoutWindow();

	QLineEdit *m_channels;
	QLineEdit *m_appName;
	QDialogButtonBox *m_buttons;
	ServiceNameValidator *m_validator;
	QLineEdit *m_priority;
};

#endif // CONSTROLSETUPDLG_H
