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

#include "DirThread.h"
#include "SyntroCFS.h"

DirThread::DirThread(QObject *, QSettings *settings)
	: m_settings(settings)
{
	m_storePath = m_settings->value(SYNTRO_CFS_STORE_PATH).toString();
}

DirThread::~DirThread()
{
}

void DirThread::initThread()
{
	m_timer = startTimer(SYNTROCFS_DM_INTERVAL);
	buildDirString();
}

void DirThread::exitThread()
{
	m_run = false;
	killTimer(m_timer);
	exit();
}

QString DirThread::getDirectory()
{
	QMutexLocker locker(&m_lock);
	return m_directory;
}

bool DirThread::processMessage(SyntroThreadMsg *msg)
{
	switch (msg->message) {
		case SYNTRO_CFS_MESSAGE:
			return true;

		case SYNTROTHREAD_TIMER_MESSAGE:
			buildDirString();
			return true;

		default:
			return true;
	}
}

void DirThread::buildDirString()
{
	QDir dir;
	QString relativePath;

	QMutexLocker locker(&m_lock);

	m_directory = "";
	relativePath = "";

	if (!dir.cd(m_storePath)) {
		logError(QString("Failed to open store path %1").arg(m_storePath));
		return;
	}


	dir.setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
	processDir(dir, m_directory, "");							// start off in root directory
	return;
}

void DirThread::processDir(QDir dir, QString& dirString, QString relativePath)
{
	QFileInfoList list = dir.entryInfoList();

	for (int i = 0; i < list.size(); i++) {
		QFileInfo fileInfo = list.at(i);
		if (fileInfo.isDir()) {
			dir.cd(fileInfo.fileName());
			if (relativePath == "")
				processDir(dir, dirString, fileInfo.fileName());
			else
				processDir(dir, dirString, relativePath + SYNTRO_SERVICEPATH_SEP + fileInfo.fileName());
			dir.cdUp();
		} else {
			if (relativePath == "")
				dirString += fileInfo.fileName() + SYNTROCFS_FILENAME_SEP;
			else
				dirString += relativePath + SYNTRO_SERVICEPATH_SEP + fileInfo.fileName() + SYNTROCFS_FILENAME_SEP;
		}
	}
}

