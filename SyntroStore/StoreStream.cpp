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

#include "SyntroUtils.h"
#include "StoreStream.h"
#include "SyntroStore.h"

// 2 GB
#define MAX_FILE_ROTATION_SIZE 0x7FFFFFFF

// 256 MB
#define DEFAULT_FILE_ROTATION_SIZE 0xFFFFFFE


StoreStream::StoreStream()
{
	m_storePath = "./";
	m_createSubFolder = false;
	m_storeFormat = structuredFileFormat;
	m_rotationPolicy = timeRotation;
	m_rotationTimeUnits = hourUnits;
	m_rotationTime = 24;
	m_rotationSize = 128;
	
	m_current = QDateTime::currentDateTime();

	clearStats();
}

StoreStream::StoreStream(const StoreStream &rhs)
{
	*this = rhs;
}

StoreStream& StoreStream::operator=(const StoreStream &rhs)
{
	if (this != &rhs) {
		m_folderWritable = rhs.m_folderWritable;
		m_streamName = rhs.m_streamName;
		m_origStorePath = rhs.m_origStorePath;
		m_storePath = rhs.m_storePath;
		m_createSubFolder = rhs.m_createSubFolder;
		m_storeFormat = rhs.m_storeFormat;
		m_rotationPolicy = rhs.m_rotationPolicy;
		m_rotationTimeUnits = rhs.m_rotationTimeUnits;
		m_rotationTime = rhs.m_rotationTime;
		m_rotationSize = rhs.m_rotationSize;

		m_currentFile = rhs.m_currentFile;
		m_currentFileFullPath = rhs.m_currentFileFullPath;
		m_currentIndexFileFullPath = rhs.m_currentIndexFileFullPath;
		m_rotationSecs = rhs.m_rotationSecs;

		m_current = rhs.m_current;

		m_rxTotalRecords = rhs.m_rxTotalRecords;
		m_rxTotalBytes = rhs.m_rxTotalBytes;
		m_rxRecords = rhs.m_rxRecords;
		m_rxBytes = rhs.m_rxBytes;
	}

	return *this;
}

QString StoreStream::pathOnly()
{
	return m_storePath;
}

QString StoreStream::streamName()
{
	return m_streamName;
}

StoreFileFormat StoreStream::storeFormat()
{
	return m_storeFormat;
}

QString StoreStream::storePath()
{
	return m_origStorePath;
}

bool StoreStream::createSubFolder()
{
	return m_createSubFolder;
}

StoreRotationPolicy StoreStream::rotationPolicy()
{
	return m_rotationPolicy;
}

StoreRotationTimeUnits StoreStream::rotationTimeUnits()
{
	return m_rotationTimeUnits;
}

qint32 StoreStream::rotationTime()
{
	return m_rotationTime;
}

qint64 StoreStream::rotationSize()
{
	return m_rotationSize;
}

void StoreStream::checkRotation()
{
	QDateTime now = QDateTime::currentDateTime();

	if (needTimeRotation(now) || needSizeRotation()) {
		m_current = now;

		m_fileMutex.lock();
		if (m_storeFormat == rawFileFormat) {
			m_currentFile = QString(m_filePrefix + m_current.toString("_yyyyMMdd_hhmm.") 
				+ SYNTRO_RECORD_FLAT_EXT);
			m_currentFileFullPath = m_storePath + m_currentFile;
		}
		else {
			m_currentFile = QString(m_filePrefix + m_current.toString("_yyyyMMdd_hhmm.") 
				+ SYNTRO_RECORD_SRF_RECORD_EXT);
			m_currentFileFullPath = m_storePath + m_currentFile;
			m_currentIndexFileFullPath = QString(m_storePath + m_filePrefix 
				+ m_current.toString("_yyyyMMdd_hhmm.") + SYNTRO_RECORD_SRF_INDEX_EXT);
		}
		m_fileMutex.unlock();

		m_statMutex.lock();
		m_rxRecords = 0;
		m_rxBytes = 0;
		m_statMutex.unlock();
	}
}

bool StoreStream::needTimeRotation(QDateTime now)
{
	if (m_rotationPolicy != timeRotation && m_rotationPolicy != anyRotation)
		return false;

	if (m_currentFileFullPath.length() == 0)
		return true;
	
	qint32 secsTo = m_current.secsTo(now);

	if (secsTo >= m_rotationSecs) {
		logDebug(QString("Switching stream " + m_streamName + " because of time"));
		return true;
	}

	return false;
}

bool StoreStream::needSizeRotation()
{
	if (m_rotationPolicy != sizeRotation && m_rotationPolicy != anyRotation)
		return false;

	if (m_currentFileFullPath.length() == 0)
		return true;

	QFileInfo info(m_currentFileFullPath);

	if (info.exists() && info.size() >= m_rotationSize) {
		logDebug(QString("Switching stream " + m_streamName + " because of size"));
		return true;
	}
	
	return false;
}

QString StoreStream::currentFile()
{
	QMutexLocker lock(&m_fileMutex);

	return m_currentFileFullPath;
}

QString StoreStream::currentFileFullPath()
{
	QMutexLocker lock(&m_fileMutex);

	return m_currentFileFullPath;
}

QString StoreStream::rawFileFullPath()
{
	return currentFileFullPath();
}

QString StoreStream::srfFileFullPath()
{
	return currentFileFullPath();
}

QString StoreStream::srfIndexFullPath()
{
	QMutexLocker lock(&m_fileMutex);
	
	return m_currentIndexFileFullPath;
}

void StoreStream::queueBlock(QByteArray block)
{
	QMutexLocker lock(&m_blockMutex);

	if (m_blocks.count() < 128)
		m_blocks.enqueue(block);

	updateStats(block.length());
}

QByteArray StoreStream::dequeueBlock()
{
	QMutexLocker lock(&m_blockMutex);
	QByteArray block;

	if (!m_blocks.empty())
		block = m_blocks.dequeue();

	return block;
}

int StoreStream::blockCount()
{
	QMutexLocker lock(&m_blockMutex);

	return m_blocks.count();
}

void StoreStream::updateStats(int recordLength)
{
	QMutexLocker lock(&m_statMutex);

	m_rxTotalRecords++;
	m_rxTotalBytes += recordLength;
	m_rxRecords++;
	m_rxBytes += recordLength;
}

qint64 StoreStream::rxTotalRecords()
{
	QMutexLocker lock(&m_statMutex);

	return m_rxTotalRecords;
}

qint64 StoreStream::rxTotalBytes()
{
	QMutexLocker lock(&m_statMutex);

	return m_rxTotalBytes;
}

qint64 StoreStream::rxRecords()
{
	QMutexLocker lock(&m_statMutex);

	return m_rxRecords;
}

qint64 StoreStream::rxBytes()
{
	QMutexLocker lock(&m_statMutex);

	return m_rxBytes;
}

void StoreStream::clearStats()
{
	QMutexLocker lock(&m_statMutex);

	m_rxTotalRecords = 0;
	m_rxTotalBytes = 0;
	m_rxRecords = 0;
	m_rxBytes = 0;
}

bool StoreStream::folderWritable()
{
	return m_folderWritable;
}

bool StoreStream::checkFolderPermissions()
{
	QFileInfo info(m_storePath);

	if (!info.exists() || !info.isDir())
		return false;

#ifdef Q_WS_WIN
	return true;
#else
	return info.isWritable();
#endif
}

// Assumed that settings has already had beginReadArray called
bool StoreStream::load(QSettings *settings)
{
	QString str;
	QDir dir;

	m_streamName = settings->value(SYNTROSTORE_PARAMS_STREAM_SOURCE).toString();
	m_filePrefix = m_streamName;
	m_filePrefix.replace("/", "_");
	m_storePath = settings->value(SYNTROSTORE_PARAMS_PATH).toString();
	if (m_storePath.length() == 0)
		m_storePath = "./";

	 if (!m_storePath.endsWith('/') && !m_storePath.endsWith('\\'))
		 m_storePath += '/';

	 m_origStorePath = m_storePath;

	if (settings->contains(SYNTROSTORE_PARAMS_CREATE_SUBFOLDER))
		m_createSubFolder = settings->value(SYNTROSTORE_PARAMS_CREATE_SUBFOLDER).toBool();

	if (m_createSubFolder)
		m_storePath += m_streamName + '/';

	if (!dir.mkpath(m_storePath)) {
		m_folderWritable = false;
		logError(QString("Failed to create folder: %1").arg(m_storePath));
	}
	else {
		m_folderWritable = checkFolderPermissions();

		if (!m_folderWritable)
			logError(QString("Folder is not writable: %1").arg(m_storePath));
	}

	if (settings->contains(SYNTROSTORE_PARAMS_FORMAT)) {
		str = settings->value(SYNTROSTORE_PARAMS_FORMAT).toString().toLower();

		if (str == SYNTRO_RECORD_STORE_FORMAT_SRF)
			m_storeFormat = structuredFileFormat;
		else if (str == SYNTRO_RECORD_STORE_FORMAT_RAW)
			m_storeFormat = rawFileFormat;
		else
			return false;
	}
	else {
		m_storeFormat = structuredFileFormat;
	}

	if (settings->contains(SYNTROSTORE_PARAMS_ROTATION_POLICY)) {
		str = settings->value(SYNTROSTORE_PARAMS_ROTATION_POLICY).toString().toLower();
		
		if (str == SYNTROSTORE_PARAMS_ROTATION_POLICY_TIME)
			m_rotationPolicy = timeRotation;
		else if (str == SYNTROSTORE_PARAMS_ROTATION_POLICY_SIZE)
			m_rotationPolicy = sizeRotation;
		else if (str == SYNTROSTORE_PARAMS_ROTATION_POLICY_ANY)
			m_rotationPolicy = anyRotation;
		else
			return false;
	}
	else {
		m_rotationPolicy = sizeRotation;
	}

	if (m_rotationPolicy == timeRotation || m_rotationPolicy == anyRotation) {
		if (settings->contains(SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS)) {
			str = settings->value(SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS).toString().toLower();

			if (str == SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS_HOURS)
				m_rotationTimeUnits = hourUnits;
			else if (str == SYNTROSTORE_PARAMS_ROTATION_TIME_UNITS_MINUTES)
				m_rotationTimeUnits = minuteUnits;
			else
				return false;
		}
		else {
			m_rotationTimeUnits = hourUnits;
		}

		if (settings->contains(SYNTROSTORE_PARAMS_ROTATION_TIME)) {
			m_rotationTime = settings->value(SYNTROSTORE_PARAMS_ROTATION_TIME).toInt();

			if (m_rotationTime < 0)
				m_rotationTime = 0;
		}
		else {
			// choose some reasonable defaults
			if (m_rotationTimeUnits == hourUnits)
				m_rotationTime = 24;
			else
				m_rotationTime = 60;
		}

		if (m_rotationTimeUnits == hourUnits)
			m_rotationSecs = m_rotationTime * 3600;
		else // if (m_rotationTimeUnits = minuteUnits)
			m_rotationSecs = m_rotationTime * 60;
	}

	if (m_rotationPolicy == sizeRotation || m_rotationPolicy == anyRotation) {
		if (settings->contains(SYNTROSTORE_PARAMS_ROTATION_SIZE)) {
			m_rotationSize = settings->value(SYNTROSTORE_PARAMS_ROTATION_SIZE).toULongLong();

			m_rotationSize *= 1000 * 1000;

			if (m_rotationSize < 0)
				m_rotationSize = 0;
			else if (m_rotationSize > MAX_FILE_ROTATION_SIZE)
				m_rotationSize = MAX_FILE_ROTATION_SIZE;
		}
		else {
			m_rotationSize = DEFAULT_FILE_ROTATION_SIZE;
		}
	}

	return true;
}
