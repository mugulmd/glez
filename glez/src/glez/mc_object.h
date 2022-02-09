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

		void set_pixel(texture* tex, unsigned int x, unsigned int y, const glm::u8vec4& color, unsigned int level = 0);
		void set_pixel(texture* tex, float x, float y, const glm::u8vec4& color, unsigned int level = 0);

		glm::u8vec4 get_pixel(texture* tex, unsigned int x, unsigned int y, unsigned int level = 0);
		glm::u8vec4 get_pixel(texture* tex, float x, float y, unsigned int level = 0);

		glm::u8vec4 get_weighted_avg(texture* tex, unsigned int x, unsigned int y, unsigned int level = 0);
	};

	class GLEZ_API mc_object : public abs_object
	{
	public:
		mc_object(unsigned int init_res = 512);
		~mc_object();

		void create_uv_layout() override;

	private:
		std::unordered_map<std::shared_ptr<quad_face>, frame> m_frames;
		unsigned int m_tex_dim;
		size_t m_max_level = 5;
		mipmap* m_mipmap;

	public:
		void add_face(std::shared_ptr<quad_face> f, unsigned int res_x, unsigned int res_y);
		void remove_face(std::shared_ptr<quad_face> f);
		inline frame& get_frame(std::shared_ptr<quad_face> f) { return m_frames[f]; }

		inline void set_max_level(size_t level) { m_max_level = level; }
		inline size_t get_max_level() { return m_max_level; }

		void pack_frames();

		inline mipmap* get_mipmap() { return m_mipmap; }
		void build_mipmap();

	public:
		void fill(std::shared_ptr<quad_face> f, const glm::u8vec4& color);
		void paint(std::shared_ptr<quad_face> f, const glm::vec2& coords, const glm::u8vec4& color);

	};

}
