#pragma once

#include <unordered_map>

#include "core.h"
#include "abs_object.h"

#include <glm/vec2.hpp>

namespace glez {

	struct GLEZ_API frame
	{
		glm::uvec2 res; // both coordinates should be powers of 2
		glm::uvec2 coord_scale;
		glm::uvec2 coord_offset;

		frame() : res(0, 0), coord_scale(0, 0), coord_offset(0, 0) {}

		frame(unsigned int res_x, unsigned int res_y) : 
			res(res_x, res_y), coord_scale(0, 0), coord_offset(0, 0)
		{}

		inline size_t size() { return res.x * res.y; }

		void set_pixel(texture* tex, unsigned int x, unsigned int y, const glm::u8vec4& color);

		glm::u8vec4 get_pixel(texture* tex, unsigned int x, unsigned int y);
	};

	class GLEZ_API mc_object : public abs_object
	{
	public:
		mc_object(unsigned int init_res = 512);
		~mc_object();

	private:
		std::unordered_map<std::shared_ptr<quad_face>, frame> m_frames;
		unsigned int m_tex_dim;

	public:
		inline frame& get_frame(std::shared_ptr<quad_face> f) { return m_frames[f]; }

		void add_face(std::shared_ptr<quad_face> f, unsigned int res_x, unsigned int res_y);

		void create_uv_layout() override;

	};

}
