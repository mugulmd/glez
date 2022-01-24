#include "glez_pch.h"

#include "scene.h"

#include <glm/vec3.hpp>

namespace glez {

	scene::scene()
	{
		m_camera = std::make_unique<camera>();
		m_camera->set_view_props(glm::vec3(3, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	}

}
