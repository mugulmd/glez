#include "global.h"
#include "log.h"

namespace glez {

	void init()
	{
		log::init();
		GLEZ_TRACE("glez successfully initialized");
	}

}
