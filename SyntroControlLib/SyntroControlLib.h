#ifndef SYNTROCONTROLLIB_H
#define SYNTROCONTROLLIB_H

#include "syntrocontrollib_global.h"

//		Timer intervals

#define	SYNTROSERVER_INTERVAL			(SYNTRO_CLOCKS_PER_SEC / 100)	// SyntroServer runs 100 times per second

#define	EXCHANGE_TIMEOUT				(5 * SYNTRO_CLOCKS_PER_SEC)	// multicast without ack timeout
#define	MULTICAST_REFRESH_TIMEOUT		(3 * SYNTRO_SERVICE_LOOKUP_INTERVAL) // time before stop passing back lookup refreshes


class SYNTROCONTROLLIB_EXPORT SyntroControlLib
{
public:
	SyntroControlLib();
	~SyntroControlLib();

private:

};

#endif // SYNTROCONTROLLIB_H
