//
//  Copyright (c) 2013 Pansenti, LLC.
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

#include "SyntroDefs.h"
#include "DirectoryEntry.h"

DirectoryEntry::DirectoryEntry(QString dirLine)
{
	m_raw = dirLine;
	parseLine();
}
	
void DirectoryEntry::setLine(QString dirLine)
{
	m_uid.clear();
	m_name.clear();
	m_type.clear();
	m_multicastService.clear();
	m_e2eService.clear();

	m_raw = dirLine;

	parseLine();
}

bool DirectoryEntry::isValid()
{
	if (m_uid.length() > 0 && m_name.length() > 0 && m_type.length() > 0)
		return true;

	return false;
}

QString DirectoryEntry::uid()
{
	return m_uid;
}

QString DirectoryEntry::appName()
{
	return m_name;
}

QString DirectoryEntry::componentType()
{
	return m_type;
}

QString DirectoryEntry::multicastService()
{
	return m_multicastService;
}

QString DirectoryEntry::e2eService()
{
	return m_e2eService;
}

void DirectoryEntry::parseLine()
{
	if (m_raw.length() == 0)
		return;

	m_uid = element(DETAG_UID);
	m_name = element(DETAG_APPNAME);
	m_type = element(DETAG_COMPTYPE);
	m_multicastService = element(DETAG_MSERVICE);
	m_e2eService = element(DETAG_ESERVICE);
}

QString DirectoryEntry::element(QString name)
{
	QString element;

	QString start= QString("<%1>").arg(name);
	
	int i = m_raw.indexOf(start);
	int j = m_raw.indexOf(QString("</%1>").arg(name));

	if (i >= 0 && j > i + start.length())
		element = m_raw.mid(i + start.length(), j - (i + start.length()));

	return element;
}
