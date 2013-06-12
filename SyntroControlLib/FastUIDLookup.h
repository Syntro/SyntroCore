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

#ifndef FASTUIDLOOKUP_H
#define FASTUIDLOOKUP_H

class FastUIDLookup
{

public:

//	Construction

	FastUIDLookup(void);
	~FastUIDLookup(void);

	//	Fast UID Lookup functions and variables

#define	FUL_LEVEL_SIZE		0x10000							// 16 bits of lookup per array

public:
	void *FULLookup(SYNTRO_UID *UID);						// looks up a UID and returns the data pointer, NULL if not found
	void FULAdd(SYNTRO_UID *UID, void *data);				// adds a UID to the fast lookup system
	void FULDelete(SYNTRO_UID *UID);						// deletes a UID from the fast lookup system

protected:
	void *FULLevel0[FUL_LEVEL_SIZE];						// the level 0 array
	QMutex m_lock;											// to ensure consistency
};

#endif // FASTUIDLOOKUP_h