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

#include "SyntroClock.h"
#include "qelapsedtimer.h"


qint64 syntroClockValue = 0;								// this contains the mS monotonic system clock
QMutex syntroClockLock;										// this is used to control access to the clock variable
QElapsedTimer syntroElapsedTimer;

void SyntroClockObject::run()
{
	m_run = true;
	syntroElapsedTimer.start();
	while (m_run)
	{
		usleep(10500);
		syntroClockLock.lock();
		syntroClockValue += syntroElapsedTimer.restart();
		syntroClockLock.unlock();
	}
}

qint64 SyntroClock()
{
	qint64 clockVal;

	syntroClockLock.lock();
	clockVal = syntroClockValue;
	syntroClockLock.unlock();
	return clockVal;
}
