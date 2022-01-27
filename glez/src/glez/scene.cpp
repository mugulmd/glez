#include "glez_pch.h"

#include "scene.h"
#include "unwrapped_object.h"

#include <glm/vec3.hpp>

namespace glez {

	scene::scene()
	{
		m_camera = new camera();
		m_camera->set_view_props(glm::vec3(3, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	}

	scene::~scene()
	{
		delete m_camera;
	}

}
