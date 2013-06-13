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

#include "SyntroStore.h"
#include "StoreDiskManager.h"

StoreDiskManager::StoreDiskManager(StoreStream *stream)
{
	m_stream = stream;
}

StoreDiskManager::~StoreDiskManager()
{
}

void StoreDiskManager::initThread()
{
	m_timerID = startTimer(SYNTRO_CLOCKS_PER_SEC);
}

void StoreDiskManager::exitThread()
{
	killTimer(m_timerID);
	exit();
}

bool StoreDiskManager::processMessage(SyntroThreadMsg *msg)
{
	switch (msg->message) {
		case SYNTROTHREAD_TIMER_MESSAGE:
			processWriteQueue();
			return true;

		default:
			return true;
	}
}

void StoreDiskManager::processWriteQueue()
{
	int blockCount;

	blockCount = m_stream->blockCount();

	if (blockCount > 0) {
		m_stream->checkRotation();

	if (m_stream->storeFormat() == structuredFileFormat)
		writeBlocksSRF(blockCount);
	else
		writeBlocksRaw(blockCount);
	}
}

void StoreDiskManager::writeBlocksSRF(int blockCount)
{
	SYNTRO_RECORD_HEADER *record;
	SYNTRO_STORE_RECORD_HEADER cHeadV0;
	qint64 pos, headerLength;
	QList<qint64> posList;

	QString dataFilename = m_stream->srfFileFullPath();
	QString indexFilename = m_stream->srfIndexFullPath();

	QFile dataFile(dataFilename);

	if (!dataFile.open(QIODevice::Append))
		return;

	QFile indexFile(indexFilename);

	if (!indexFile.open(QIODevice::Append)) {
		dataFile.close();
		return;
	}

	strncpy(cHeadV0.sync, SYNC_STRINGV0, SYNC_LENGTH);
	SyntroUtils::convertIntToUC4(0, cHeadV0.data);

	for (int i = 0; i < blockCount; i++) {
		QByteArray block = m_stream->dequeueBlock();

		SyntroUtils::convertIntToUC4(block.size(), cHeadV0.size);
		
		record = reinterpret_cast<SYNTRO_RECORD_HEADER *>(block.data());

		dataFile.write((char *)(&cHeadV0), sizeof(SYNTRO_STORE_RECORD_HEADER));
		headerLength = (qint64)sizeof(SYNTRO_STORE_RECORD_HEADER);

		pos = dataFile.pos();
		pos -= headerLength;

		dataFile.write((char *)record, block.size());
		posList.append(pos);
	}

	for (int i = 0; i < posList.count(); i++) {
		pos = posList[i];
		indexFile.write((char *)&pos, sizeof(qint64));
	}
	
	indexFile.close();
	dataFile.close();
}

void StoreDiskManager::writeBlocksRaw(int blockCount)
{
	SYNTRO_RECORD_HEADER *record;
	int headerLen;

	QString dataFilename = m_stream->rawFileFullPath();

	QFile rf(dataFilename);

	if (!rf.open(QIODevice::Append)) {
		logWarn(QString("Failed opening file %1").arg(dataFilename));
		return;
	}

	for (int i = 0; i < blockCount; i++) {
		QByteArray block = m_stream->dequeueBlock();

		record = reinterpret_cast<SYNTRO_RECORD_HEADER *>(block.data());
		headerLen = SyntroUtils::convertUC2ToInt(record->headerLength);

		if (headerLen < 0 || headerLen > block.size()) {
			logWarn(QString("SyntroRecordWriteFlat - invalid header size %1").arg(headerLen));
			continue;
		}

		rf.write((char *)record + headerLen, block.size() - headerLen);
	}

	rf.close();
}

