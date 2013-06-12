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

#ifndef STORESTREAM_H
#define STORESTREAM_H

#include <qsettings.h>
#include <qmutex.h>
#include <qdatetime.h>
#include <qqueue.h>
#include <qlabel.h>


enum StoreFileFormat { structuredFileFormat, rawFileFormat };
enum StoreRotationPolicy { timeRotation, sizeRotation, anyRotation };
enum StoreRotationTimeUnits { minuteUnits, hourUnits };  


class StoreStream
{
public:
	StoreStream();
	StoreStream(const StoreStream &rhs);

	StoreStream& operator=(const StoreStream &rhs);

	QString pathOnly();
	QString streamName();
	StoreFileFormat storeFormat();
	QString storePath();
	bool createSubFolder();
	StoreRotationPolicy rotationPolicy();
	StoreRotationTimeUnits rotationTimeUnits();
	qint32 rotationTime();
	qint64 rotationSize();

	bool folderWritable();
	void checkRotation();

	QString currentFile();
	QString currentFileFullPath();
	QString rawFileFullPath();
	QString srfFileFullPath();
	QString srfIndexFullPath();

	void updateStats(int recordLength);
	qint64 rxTotalRecords();
	qint64 rxTotalBytes();
	qint64 rxRecords();
	qint64 rxBytes();
	void clearStats();

	bool load(QSettings *settings);

	void queueBlock(QByteArray block);
	QByteArray dequeueBlock();
	int blockCount();
	int port;

private:
	bool needTimeRotation(QDateTime now);
	bool needSizeRotation();
	bool checkFolderPermissions();

	bool m_folderWritable;
	QString m_streamName;
	QString m_filePrefix;
	QString m_storePath;
	QString m_origStorePath;
	bool m_createSubFolder;
	StoreFileFormat m_storeFormat;
	StoreRotationPolicy m_rotationPolicy;
	StoreRotationTimeUnits m_rotationTimeUnits;
	qint32 m_rotationTime;
	qint64 m_rotationSize;

	QString m_currentFile;
	QString m_currentFileFullPath;
	QString m_currentIndexFileFullPath;
	qint32 m_rotationSecs;

	QMutex m_fileMutex;
	QDateTime m_current;

	QMutex m_statMutex;
	qint64 m_rxTotalRecords;
	qint64 m_rxTotalBytes;
	qint64 m_rxRecords;
	qint64 m_rxBytes;

	QMutex m_blockMutex;
	QQueue<QByteArray> m_blocks;
};

#endif

