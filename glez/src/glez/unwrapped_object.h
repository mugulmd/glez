#pragma once

#include <memory>
#include <unordered_map>
#include <array>

#include "core.h"
#include "abs_object.h"

#include <glm/vec2.hpp>

namespace glez {

	class GLEZ_API unwrapped_object : public abs_object
	{
	public:
		unwrapped_object(texture* _texture);
		~unwrapped_object();

		void create_uv_layout() override;

	private:
		std::unordered_map<std::shared_ptr<half_edge>, glm::vec2> m_tex_coords;

	public:
		inline glm::vec2& get_tex_coord(std::shared_ptr<half_edge> f) { return m_tex_coords[f]; }
		void add_face(std::shared_ptr<quad_face> face, std::array<glm::vec2, 4> tex_coords);

	};

}
