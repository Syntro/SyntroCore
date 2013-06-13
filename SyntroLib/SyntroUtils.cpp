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

#include "LogWrapper.h"
#include "SyntroDefs.h"
#include "SyntroUtils.h"
#include "SyntroSocket.h"
#include "SyntroClock.h"
#include "Endpoint.h"
#include <qfileinfo.h>
#include <qdir.h>
#include <qhostinfo.h>

//	globals

SYNTRO_IPADDR myIPAddr;										// the IP address
SYNTRO_MACADDR myMacAddr;									// the mac address

//	The actual SyntroClock generator

SyntroClockObject *syntroClockGen;							// the actual clock generator

//	The address info for the adaptor being used

QHostAddress platformBroadcastAddress;					
QHostAddress platformSubnetAddress;					
QHostAddress platformNetMask;					

SYNTRO_IPADDR *getMyIPAddr()
{
	return &myIPAddr;
}

SYNTRO_MACADDR *getMyMacAddr()
{
	return &myMacAddr;
}


const char *syntroLibVersion()
{
	return SYNTROLIB_VERSION;
}


void syntroAppInit(QSettings *settings)
{
	syntroClockGen = new SyntroClockObject();
	syntroClockGen->start();
	logCreate(settings);
	getMyIPAddress(settings);
}

void syntroAppExit()
{
	logDestroy();
	if (syntroClockGen) {
		syntroClockGen->m_run = false;
		syntroClockGen->wait(200);							// should never take more than this
		delete syntroClockGen;
		syntroClockGen = NULL;
	}
}


//	checkConsoleModeFlag check for "-c" in the runtime args and returns true if found

bool checkConsoleModeFlag(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-c"))
			return true;
	}

	return false;
}

bool checkDaemonModeFlag(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-d"))
            return true;
    }

    return false;
}

bool isSendOK(unsigned char seq, unsigned char ack)
{
	return (seq - ack) < SYNTRO_MAX_WINDOW;
}


void makeUIDSTR(SYNTRO_UIDSTR UIDStr, SYNTRO_MACADDRSTR macAddress, int instance)
{
	sprintf(UIDStr, "%s%04x", macAddress, instance);
}

//	UIDSTRtoUID converts a string UID to a binary one

void UIDSTRtoUID(SYNTRO_UIDSTR sourceStr, SYNTRO_UID *destUID)
{
	int byteTemp;

	for (int i = 0; i < SYNTRO_MACADDR_LEN; i++) {
		sscanf(sourceStr + i * 2, "%02X", &byteTemp);
		destUID->macAddr[i] = byteTemp;
	}

	sscanf(sourceStr + SYNTRO_MACADDR_LEN * 2, "%04X", &byteTemp);
	convertIntToUC2(byteTemp, destUID->instance);
}

//	UIDtoUIDSTR converts a binary UID to a string one

void UIDtoUIDSTR(SYNTRO_UID *sourceUID, SYNTRO_UIDSTR destStr)
{
	for (int i = 0; i < SYNTRO_MACADDR_LEN; i++)
		sprintf(destStr + i * 2, "%02X", sourceUID->macAddr[i]);

	sprintf(destStr + SYNTRO_MACADDR_LEN * 2, "%04x", convertUC2ToUInt(sourceUID->instance));
}

//	displayIPAddr - returns a string version of UP address for displaying

QString displayIPAddr(SYNTRO_IPADDR address)
{
	return QString("%1.%2.%3.%4").arg(address[0]).arg(address[1]).arg(address[2]).arg(address[3]);
}

//	convertIPStringToIPAddr converts a string form IP address (with dots) to the internal form

void convertIPStringToIPAddr(char *IPStr, SYNTRO_IPADDR IPAddr)
{
	int	a[4];

	sscanf(IPStr, "%d.%d.%d.%d", a, a+1, a+2, a+3);
	
	for (int i = 0; i < 4; i++)
		IPAddr[i] = (unsigned char)a[i];
}

bool IPZero(SYNTRO_IPADDR addr)
{
	return (addr[0] == 0) && (addr[1] == 0) && (addr[2] == 0) && (addr[3] == 0);
}

bool IPLoopback(SYNTRO_IPADDR addr)
{
	return (addr[0] == 127) && (addr[1] == 0) && (addr[2] == 0) && (addr[3] == 1);
}


//	displayUID - returns a string version of UID for displaying

QString displayUID(SYNTRO_UID *uid)
{
	char temp[20];

	UIDtoUIDSTR(uid, temp);

	return QString(temp);
}

bool compareUID(SYNTRO_UID *a, SYNTRO_UID *b)
{
	return memcmp(a, b, sizeof(SYNTRO_UID)) == 0;
}

bool UIDHigher(SYNTRO_UID *a, SYNTRO_UID *b)
{
	return memcmp(a, b, sizeof(SYNTRO_UID)) > 0;
}

//	swapEHead swaps UIDs and ports in a SYNTRO_EHEAD

void swapEHead(SYNTRO_EHEAD *ehead)
{
	swapUID(&(ehead->sourceUID), &(ehead->destUID));

	int i = convertUC2ToInt(ehead->sourcePort);

	copyUC2(ehead->sourcePort, ehead->destPort);

	convertIntToUC2(i, ehead->destPort);
}

void swapUID(SYNTRO_UID *a, SYNTRO_UID *b)
{
	SYNTRO_UID temp;

	memcpy(&temp, a, sizeof(SYNTRO_UID));
	memcpy(a, b, sizeof(SYNTRO_UID));
	memcpy(b, &temp, sizeof(SYNTRO_UID));
}

//	UC2 and UC4 Conversion routines
//
//	*** Note: 32 bit int assumed ***
//

int	convertUC4ToInt(SYNTRO_UC4 uc4)
{
	return ((int)uc4[0] << 24) | ((int)uc4[1] << 16) | ((int)uc4[2] << 8) | uc4[3];
}

void convertIntToUC4(int val, SYNTRO_UC4 uc4)
{
	uc4[3] = val & 0xff;
	uc4[2] = (val >> 8) & 0xff;
	uc4[1] = (val >> 16) & 0xff;
	uc4[0] = (val >> 24) & 0xff;
}

int	convertUC2ToInt(SYNTRO_UC2 uc2)
{
	int val = ((int)uc2[0] << 8) | uc2[1];
	
	if (val & 0x8000)
		val |= 0xffff0000;
	
	return val;
}

int convertUC2ToUInt(SYNTRO_UC2 uc2)
{
	return ((int)uc2[0] << 8) | uc2[1];
}

void convertIntToUC2(int val, SYNTRO_UC2 uc2)
{
	uc2[1] = val & 0xff;
	uc2[0] = (val >> 8) & 0xff;
}

void copyUC2(SYNTRO_UC2 dst, SYNTRO_UC2 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
}


SYNTRO_EHEAD *createEHEAD(SYNTRO_UID *sourceUID, int sourcePort, SYNTRO_UID *destUID, int destPort, 
										unsigned char seq, int len)
{
	SYNTRO_EHEAD *ehead;

	ehead = (SYNTRO_EHEAD *)malloc(len + sizeof(SYNTRO_EHEAD));
	memcpy(&(ehead->sourceUID), sourceUID, sizeof(SYNTRO_UID));
	convertIntToUC2(sourcePort, ehead->sourcePort);									
	memcpy(&(ehead->destUID), destUID, sizeof(SYNTRO_UID));
	convertIntToUC2(destPort, ehead->destPort);									
	ehead->seq = seq;

	return ehead;
}


bool crackServicePath(QString servicePath, QString& regionName, QString& componentName, QString& serviceName)
{
	regionName.clear();
	componentName.clear();
	serviceName.clear();
 
	QStringList stringList = servicePath.split(SYNTRO_SERVICEPATH_SEP);

	if (stringList.count() > 3) {
		logWarn(QString("Service path received has invalid format ") + servicePath);
		return false;
	}
	switch (stringList.count()) {
		case 1:
			serviceName = stringList.at(0);
			break;

		case 2:
			serviceName = stringList.at(1);
			componentName = stringList.at(0);
			break;

		case 3:
			serviceName = stringList.at(2);
			componentName = stringList.at(1);
			regionName = stringList.at(0);
	}
	return true;
}

//	loadStandardSettings looks for the following command line parameters in any order
//
//		-s<settings-file-path>		- default is ./<COMPONENT_TYPE>.ini
//		-a<network-adapter>			- default is first with valid IP address

QSettings *loadStandardSettings(const char *appType, QStringList arglist)
{
	QString args;
	QString adapter;									// IP adaptor name
	QString settingsPath;								// the settings file path
	QSettings *settings = NULL;

	settingsPath = "";
	adapter = "";

	for (int i = 1; i < arglist.size(); i++) {			// skip the first string as that is the program name
		QString opt = arglist.at(i);					// get the param

		if (opt.at(0).toLatin1() != '-')
			continue;									// doesn't start with '-'

		char optCode = opt.at(1).toLatin1();				// get the option code
		opt.remove(0, 2);								// remove the '-' and the code

		switch (optCode) {
		case 's':
			settingsPath = opt;
			break;

		case 'a':
			adapter = opt;
			break;

		case 'c':
		// console app flag, handled elsewhere
			break;

        case 'd':
        // daemon mode flag, handled elsewhere
            break;

        default:
			logWarn(QString("Unrecognized argument option %1").arg(optCode));
		}
	}

	// user can override the default location for the ini file

    if (settingsPath.size() != 0)
		settings = new QSettings(settingsPath, QSettings::IniFormat);
    else
		settings = new QSettings(QDir::currentPath() + "/" + appType + ".ini", QSettings::IniFormat);

	// Save settings generated from command line arguments

	settings->setValue(SYNTRO_RUNTIME_PATH, settingsPath);

	if (adapter.length() > 0) {
		settings->setValue(SYNTRO_RUNTIME_ADAPTER, adapter);
	} else {
		// we don't want to overwrite an existing entry so check first
		adapter = settings->value(SYNTRO_RUNTIME_ADAPTER).toString();

		// but we still want to put a default placeholder
		if (adapter.length() < 1)
			settings->setValue(SYNTRO_RUNTIME_ADAPTER, adapter);
	}

	//	Make sure common settings are present or else generate defaults

	if (!settings->contains(SYNTRO_PARAMS_APPNAME))
		settings->setValue(SYNTRO_PARAMS_APPNAME, QHostInfo::localHostName());		// use hostname for app name
	//	Force type to be mine always

	settings->setValue(SYNTRO_PARAMS_APPTYPE, appType);

	//	Add in SyntroControl array

 	int	size = settings->beginReadArray(SYNTRO_PARAMS_CONTROL_NAMES);
	settings->endArray();

	if (size == 0) {
		settings->beginWriteArray(SYNTRO_PARAMS_CONTROL_NAMES);

		for (int control = 0; control < ENDPOINT_MAX_SYNTROCONTROLS; control++) {
			settings->setArrayIndex(control);
			settings->setValue(SYNTRO_PARAMS_CONTROL_NAME, "");
		}
		settings->endArray();
	}

	if (!settings->contains(SYNTRO_PARAMS_CONTROLREVERT))
		settings->setValue(SYNTRO_PARAMS_CONTROLREVERT, 0);		
	
	if (!settings->contains(SYNTRO_PARAMS_LOCALCONTROL))
		settings->setValue(SYNTRO_PARAMS_LOCALCONTROL, 0);		
	
	if (!settings->contains(SYNTRO_PARAMS_LOCALCONTROL_PRI))
		settings->setValue(SYNTRO_PARAMS_LOCALCONTROL_PRI, 0);		
	
	if (!settings->contains(SYNTRO_PARAMS_HBINTERVAL))
		settings->setValue(SYNTRO_PARAMS_HBINTERVAL, SYNTRO_HEARTBEAT_INTERVAL);

	if (!settings->contains(SYNTRO_PARAMS_HBTIMEOUT))
		settings->setValue(SYNTRO_PARAMS_HBTIMEOUT, SYNTRO_HEARTBEAT_TIMEOUT);

	return settings;
}

bool isReservedNameCharacter(char value)
{
	switch (value) {
		case SYNTROCFS_FILENAME_SEP:
		case SYNTRO_STREAM_TYPE_SEP:
		case SYNTRO_SERVICEPATH_SEP:
		case SYNTRO_LOG_COMPONENT_SEP:
		case ' ' :
		case '\\':
			return true;

		default:
			return false;
	}
}

bool isReservedPathCharacter(char value)
{
	switch (value) {
		case SYNTROCFS_FILENAME_SEP:
		case SYNTRO_STREAM_TYPE_SEP:
		case SYNTRO_LOG_COMPONENT_SEP:
		case ' ' :
		case '\\':
			return true;

		default:
			return false;
	}
}

QHostAddress getMyBroadcastAddress()
{
	return platformBroadcastAddress;
}

QHostAddress getMySubnetAddress()
{
	return platformSubnetAddress;
}

QHostAddress getMyNetMask()
{
	return platformNetMask;
}

bool isInMySubnet(SYNTRO_IPADDR IPAddr)
{
	quint32 intaddr;

	if ((IPAddr[0] == 0) && (IPAddr[1] == 0) && (IPAddr[2] == 0) && (IPAddr[3] == 0))
		return true;
	if ((IPAddr[0] == 0xff) && (IPAddr[1] == 0xff) && (IPAddr[2] == 0xff) && (IPAddr[3] == 0xff))
		return true;
	intaddr = (IPAddr[0] << 24) + (IPAddr[1] << 16) + (IPAddr[2] << 8) + IPAddr[3];
	return (intaddr & platformNetMask.toIPv4Address()) == platformSubnetAddress.toIPv4Address(); 
}


bool syntroTimerExpired(qint64 now, qint64 start, qint64 interval)
{
	if (interval == 0)
		return false;										// can never expire
	return ((now - start) >= interval);
}

void getMyIPAddress(QSettings *settings)
{
	QNetworkInterface		cInterface;
	QNetworkAddressEntry	cEntry;
	QList<QNetworkAddressEntry>	cList;
	QHostAddress			haddr;
	quint32 intaddr;
	quint32 intmask;
	int addr[SYNTRO_MACADDR_LEN];

	while (1) {
		QList<QNetworkInterface> ani = QNetworkInterface::allInterfaces();
		foreach (cInterface, ani) {
			QString name = cInterface.humanReadableName();
			logDebug(QString("Found IP adaptor %1").arg(name));

			if ((strcmp(qPrintable(name), qPrintable(settings->value(SYNTRO_RUNTIME_ADAPTER).toString())) == 0) || 
				(strlen(qPrintable(settings->value(SYNTRO_RUNTIME_ADAPTER).toString())) == 0)) {
				cList = cInterface.addressEntries();				// check to see if there's an address
				if (cList.size() > 0) {
					foreach (cEntry, cList) {
						haddr = cEntry.ip();
						intaddr = haddr.toIPv4Address();			// get it as four bytes
						if (intaddr == 0)
							continue;								// not real
	
						myIPAddr[0] = intaddr >> 24;
						myIPAddr[1] = intaddr >> 16;
						myIPAddr[2] = intaddr >> 8;
						myIPAddr[3] = intaddr;
						if (IPLoopback(myIPAddr) || IPZero(myIPAddr))
							continue;
						QString macaddr = cInterface.hardwareAddress();					// get the MAC address
						sscanf(macaddr.toLocal8Bit().constData(), "%x:%x:%x:%x:%x:%x", 
							addr, addr+1, addr+2, addr+3, addr+4, addr+5);

						for (int i = 0; i < SYNTRO_MACADDR_LEN; i++)
							myMacAddr[i] = addr[i];

						logInfo(QString("Using IP adaptor %1").arg(displayIPAddr(myIPAddr)));

						platformNetMask = cEntry.netmask();
						intmask = platformNetMask.toIPv4Address();
						intaddr &= intmask;
						platformSubnetAddress = QHostAddress(intaddr);
						intaddr |= ~intmask;
						platformBroadcastAddress = QHostAddress(intaddr);
						logInfo(QString("Subnet = %1, netmask = %2, bcast = %3")
							.arg(platformSubnetAddress.toString())
							.arg(platformNetMask.toString())
							.arg(platformBroadcastAddress.toString()));

						return;
					}
				}
			}
		}

		TRACE1("Waiting for adapter %s", qPrintable(settings->value(SYNTRO_RUNTIME_ADAPTER).toString()));
		QThread::yieldCurrentThread();
	}
}

void setSyntroTimestamp(SYNTRO_TIMESTAMP *timestamp)
{
	QDateTime	tm = QDateTime::currentDateTime();
	QDate tmd = tm.date();
	QTime tmt = tm.time();

	convertIntToUC2(tmd.year(), timestamp->year);
	convertIntToUC2(tmd.month(), timestamp->month);
	convertIntToUC2(tmd.dayOfWeek(), timestamp->dayOfWeek);
	convertIntToUC2(tmd.day(), timestamp->day);
	convertIntToUC2(tmt.hour(), timestamp->hour);
	convertIntToUC2(tmt.minute(), timestamp->minute);
	convertIntToUC2(tmt.second(), timestamp->second);
	convertIntToUC2(tmt.msec(), timestamp->milliseconds);
}

QString timestampToString(SYNTRO_TIMESTAMP *timestamp)
{
	QString str;

	str.sprintf("%d/%02d/%02d %02d:%02d:%02d.%03d", 
		convertUC2ToInt(timestamp->year),
		convertUC2ToInt(timestamp->month),
		convertUC2ToInt(timestamp->day),
		convertUC2ToInt(timestamp->hour),
		convertUC2ToInt(timestamp->minute),
		convertUC2ToInt(timestamp->second),
		convertUC2ToInt(timestamp->milliseconds));
	return str;
}

QString timestampToString(QDateTime *timestamp)
{
	QString str;

	QDate tmd = timestamp->date();
	QTime tmt = timestamp->time();

	str.sprintf("%d/%02d/%02d %02d:%02d:%02d.%03d", 
		tmd.year(),
		tmd.month(),
		tmd.day(),
		tmt.hour(),
		tmt.minute(),
		tmt.second(),
		tmt.msec());
	return str;
}

QString timestampToDateString(SYNTRO_TIMESTAMP *timestamp)
{
	QString str;

	str.sprintf("%d/%02d/%02d", 
			convertUC2ToInt(timestamp->year),
			convertUC2ToInt(timestamp->month),
			convertUC2ToInt(timestamp->day));

	return str;
}

QString timestampToTimeString(SYNTRO_TIMESTAMP *timestamp)
{
	QString str;

	str.sprintf("%02d:%02d:%02d.%03d", 
			convertUC2ToInt(timestamp->hour),
			convertUC2ToInt(timestamp->minute),
			convertUC2ToInt(timestamp->second),
			convertUC2ToInt(timestamp->milliseconds));

	return str;
}


void getSyntroTimestamp(SYNTRO_TIMESTAMP *timestamp, QDateTime *time)
{
	time->setTime(QTime(convertUC2ToUInt(timestamp->hour), convertUC2ToUInt(timestamp->minute), 
			convertUC2ToUInt(timestamp->second), convertUC2ToUInt(timestamp->milliseconds)));

	time->setDate(QDate(convertUC2ToUInt(timestamp->year), convertUC2ToUInt(timestamp->month),
		convertUC2ToUInt(timestamp->milliseconds)));
}

QDateTime syntroTimestampToQDateTime(SYNTRO_TIMESTAMP *timestamp)
{
	QDate d(convertUC2ToInt(timestamp->year), convertUC2ToInt(timestamp->month), 
			convertUC2ToInt(timestamp->day));

	QTime t(convertUC2ToInt(timestamp->hour), convertUC2ToInt(timestamp->minute), 
			convertUC2ToInt(timestamp->second), convertUC2ToInt(timestamp->milliseconds));

	return QDateTime(d, t);
}
