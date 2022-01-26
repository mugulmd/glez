#pragma once

#include "core.h"

#include <glm/vec3.hpp>

namespace glez {

	struct GLEZ_API ray
	{
		glm::vec3 origin;
		glm::vec3 direction;

		ray(glm::vec3 _origin, glm::vec3 _direction);

		inline glm::vec3 at(float t) const { return origin + direction * t; }
	};

}
