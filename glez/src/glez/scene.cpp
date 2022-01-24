#include "scene.h"

#include "log.h"

namespace glez {

	scene::scene()
	{
		log::init();
		GLEZ_INFO("created new scene");
	}

	scene::~scene()
	{
		GLEZ_INFO("deleted scene");
	}

}
