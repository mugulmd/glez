#include "glez_pch.h"

#include "mc_object.h"

namespace glez {

	void frame::set_pixel(texture* tex, unsigned int x, unsigned int y, const glm::u8vec4& color, unsigned int level)
	{
		unsigned int scale = std::pow(2, level);
		unsigned int scale_x = std::min(scale, res.x);
		unsigned int scale_y = std::min(scale, res.y);
		tex->set_pixel(tex->height() - 1 - ((coord_scale.y / scale_y) + coord_offset.y + y), 
			(coord_scale.x / scale_x) + coord_offset.x + x, 
			color);
	}

	glm::u8vec4 frame::get_pixel(texture* tex, unsigned int x, unsigned int y, unsigned int level)
	{
		unsigned int scale = std::pow(2, level);
		unsigned int scale_x = std::min(scale, res.x);
		unsigned int scale_y = std::min(scale, res.y);
		return tex->get_pixel(tex->height() - 1 - ((coord_scale.y / scale_y) + coord_offset.y + y),
			(coord_scale.x / scale_x) + coord_offset.x + x);
	}

	glm::u8vec4 frame::get_weighted_avg(texture* tex, unsigned int x, unsigned int y, unsigned int level)
	{
		unsigned int scale = std::pow(2, level);
		unsigned int scale_x = std::min(scale, res.x);
		unsigned int scale_y = std::min(scale, res.y);

		glm::u8vec4 c0 = get_pixel(tex, x, y, level);

		float w1 = 0.f;
		glm::u8vec4 c1(0, 0, 0, 0);
		if (x > 0) {
			c1 += get_pixel(tex, x - 1, y, level); w1 += 0.5f;
		}
		if (x < res.x / scale_x) {
			c1 += get_pixel(tex, x + 1, y, level); w1 += 0.5f;
		}
		if (y > 0) {
			c1 += get_pixel(tex, x, y - 1, level); w1 += 0.5f;
		}
		if (y < res.y / scale_y) {
			c1 += get_pixel(tex, x, y + 1, level); w1 += 0.5f;
		}

		float w2 = 0.f;
		glm::u8vec4 c2(0, 0, 0, 0);
		if (x > 0 && y > 0) {
			c2 += get_pixel(tex, x - 1, y - 1, level); w2 += 0.25f;
		}
		if (x > 0 && y < res.y / scale_y) {
			c2 += get_pixel(tex, x - 1, y + 1, level); w2 += 0.25f;
		}
		if (x < res.x / scale_x && y > 0) {
			c2 += get_pixel(tex, x + 1, y - 1, level); w2 += 0.25f;
		}
		if (x < res.x / scale_x && y < res.y / scale_y) {
			c2 += get_pixel(tex, x + 1, y + 1, level); w2 += 0.25f;
		}

		return glm::u8vec4((glm::vec4(c0) + 0.5f * glm::vec4(c1) + 0.25f * glm::vec4(c2)) / (w1 + w2));
	}

	mc_object::mc_object(unsigned int init_res) :
		abs_object(new texture(init_res, init_res), 4), 
		m_tex_dim(init_res), 
		m_mipmap(new mipmap())
	{}

	mc_object::~mc_object()
	{
		delete m_mipmap;
	}

	void mc_object::add_face(std::shared_ptr<quad_face> f, unsigned int res_x, unsigned int res_y)
	{
		m_mesh->add_face(f);
		m_frames[f] = frame(res_x, res_y);
	}

	void mc_object::pack_frames()
	{
		// sort faces by frame size in reverse order
		std::list<std::shared_ptr<quad_face>>& faces = m_mesh->get_faces();
		faces.sort([this](const std::shared_ptr<quad_face>& f1, const std::shared_ptr<quad_face>& f2) {
			return m_frames[f1].size() < m_frames[f2].size();
		});
		faces.reverse();

		// packing algorithm
		m_tex_dim = 512;
		int b = 2;
		while (true) {
			glm::uvec2 u_s(0, 0);
			glm::uvec2 u_delta(0, 0);
			unsigned int row_height = 0;
			bool success = true;
			for (std::shared_ptr<quad_face>& f : faces) {
				// reach end of row
				if (u_s.x + u_delta.x + m_frames[f].res.x >= m_tex_dim - 1) {
					// go to next available row
					u_s.x = 0;
					u_s.y += row_height;
					u_delta.x = 0;
					u_delta.y += b;
					row_height = 0;
				}
				// check if texture size is not too small
				if (u_s.y + u_delta.y + m_frames[f].res.y >= m_tex_dim - 1) {
					// take a bigger texture and restart packing
					m_tex_dim *= 2;
					success = false;
					break;
				}
				// place texture coordinates for each quad corner
				m_frames[f].coord_scale = u_s;
				m_frames[f].coord_offset = u_delta;
				// move to next position
				u_s.x += m_frames[f].res.x;
				u_delta.x += b;
				row_height = std::max(row_height, m_frames[f].res.y);
			}
			if (success) {
				break;
			}
		}
		GLEZ_INFO("mesh color texture packing done: texture dim={}", m_tex_dim);

		delete m_texture;
		m_texture = new texture(m_tex_dim, m_tex_dim);
	}

	void mc_object::create_uv_layout()
	{
		m_buffer->init_store(m_mesh->get_faces().size());

		for (std::shared_ptr<quad_face>& f : m_mesh->get_faces()) {
			frame& fr = get_frame(f);
			std::vector<float> face_uv(16);
			unsigned int order[4][2]{
				{0, 0}, {1, 0}, {1, 1}, {0, 1}
			};
			for (size_t i = 0; i < 4; i++) {
				face_uv[4 * i] = (float)(fr.coord_scale.x + order[i][0] * fr.res.x) 
					/ (float)m_texture->width();
				face_uv[4 * i + 1] = (float)(fr.coord_scale.y + order[i][1] * fr.res.y) 
					/ (float)m_texture->height();
				face_uv[4 * i + 2] = ((float)fr.coord_offset.x + 0.5f)
					/ (float)m_texture->width();
				face_uv[4 * i + 3] = ((float)fr.coord_offset.y + 0.5f)
					/ (float)m_texture->height();
			}
			m_buffer->store_face(f, face_uv);
		}

		notify_render_buffer_listeners();
	}

	void mc_object::build_mipmap(unsigned int max_level)
	{
		delete m_mipmap;
		m_mipmap = new mipmap(m_texture);

		for (unsigned int level = 1; level <= max_level; level++) {
			texture* tex_prev = m_mipmap->levels.back();

			unsigned int scale = std::pow(2, level);
			unsigned int dim_x = 1;
			unsigned int dim_y = 1;
			for (std::shared_ptr<quad_face>& f : m_mesh->get_faces()) {
				frame& fr = get_frame(f);
				unsigned int scale_x = std::min(scale, fr.res.x);
				unsigned int scale_y = std::min(scale, fr.res.y);
				dim_x = std::max(dim_x, (fr.coord_scale.x + fr.res.x) / scale_x + fr.coord_offset.x);
				dim_y = std::max(dim_y, (fr.coord_scale.y + fr.res.y) / scale_y + fr.coord_offset.y);
			}
			texture* tex = m_mipmap->add_level(dim_x, dim_y);

			for (std::shared_ptr<quad_face>& f : m_mesh->get_faces()) {
				frame& fr = get_frame(f);
				unsigned int scale_x = std::min(scale, fr.res.x);
				unsigned int scale_y = std::min(scale, fr.res.y);
				// vertex colors
				{
					unsigned int x = 0;
					unsigned int y = 0;
					fr.set_pixel(tex, x, y, fr.get_weighted_avg(tex_prev, 2 * x, 2 * y, level - 1), level);
				}
				{
					unsigned int x = 0;
					unsigned int y = fr.res.y / scale_y;
					fr.set_pixel(tex, x, y, fr.get_weighted_avg(tex_prev, 2 * x, 2 * y, level - 1), level);
				}
				{
					unsigned int x = fr.res.x / scale_x;
					unsigned int y = 0;
					fr.set_pixel(tex, x, y, fr.get_weighted_avg(tex_prev, 2 * x, 2 * y, level - 1), level);
				}
				{
					unsigned int x = fr.res.x / scale_x;
					unsigned int y = fr.res.y / scale_y;
					fr.set_pixel(tex, x, y, fr.get_weighted_avg(tex_prev, 2 * x, 2 * y, level - 1), level);
				}
				// edge colors
				for (unsigned int x = 1; x < fr.res.x / scale_x; x++) {
					unsigned int y = 0;
					fr.set_pixel(tex, x, y, fr.get_weighted_avg(tex_prev, 2 * x, 2 * y, level - 1), level);
				}
				for (unsigned int x = 1; x < fr.res.x / scale_x; x++) {
					unsigned int y = fr.res.y / scale_y;
					fr.set_pixel(tex, x, y, fr.get_weighted_avg(tex_prev, 2 * x, 2 * y, level - 1), level);
				}
				for (unsigned int y = 1; y < fr.res.y / scale_y; y++) {
					unsigned int x = 0;
					fr.set_pixel(tex, x, y, fr.get_weighted_avg(tex_prev, 2 * x, 2 * y, level - 1), level);
				}
				for (unsigned int y = 1; y < fr.res.y / scale_y; y++) {
					unsigned int x = fr.res.x / scale_x;
					fr.set_pixel(tex, x, y, fr.get_weighted_avg(tex_prev, 2 * x, 2 * y, level - 1), level);
				}
				// face colors
				for (unsigned int x = 1; x < fr.res.x / scale_x; x++) {
					for (unsigned int y = 1; y < fr.res.y / scale_y; y++) {
						fr.set_pixel(tex, x, y, fr.get_weighted_avg(tex_prev, 2 * x, 2 * y, level - 1), level);
					}
				}
			}
		}
	}

}
