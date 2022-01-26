#pragma once

#include <memory>
#include <unordered_map>

#include "core.h"
#include "abs_object.h"

#include <glm/vec2.hpp>

namespace glez {

	class GLEZ_API unwrapped_object : public abs_object
	{
	public:
		unwrapped_object(quad_mesh& _mesh, texture& _texture);

		void unwrap(std::unordered_map<std::shared_ptr<half_edge>, glm::vec2> tex_coords);

	};

}
