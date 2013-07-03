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

#ifndef SYNTROCONTROLLIB_GLOBAL_H
#define SYNTROCONTROLLIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined Q_OS_WIN
	#if defined SYNTROCONTROL_STATIC_LIB
		#define SYNTROCONTROLLIB_EXPORT
    #elif defined SYNTROCONTROLLIB_LIB
        #define SYNTROCONTROLLIB_EXPORT Q_DECL_EXPORT
    #else
        #define SYNTROCONTROLLIB_EXPORT Q_DECL_IMPORT
    #endif
#else
    #define SYNTROCONTROLLIB_EXPORT
#endif

#endif // SYNTROCONTROLLIB_GLOBAL_H
