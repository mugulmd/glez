#include "glez_pch.h"

#include "mc_object.h"

namespace glez {

	void frame::set_pixel(texture* tex, unsigned int x, unsigned int y, const glm::u8vec4& color)
	{
		tex->set_pixel(tex->height() - 1 - (coord_scale.y + coord_offset.y + y), 
			coord_scale.x + coord_offset.x + x, 
			color);
	}

	glm::u8vec4 frame::get_pixel(texture* tex, unsigned int x, unsigned int y)
	{
		return tex->get_pixel(coord_scale.x + coord_offset.x + x,
			coord_scale.y + coord_offset.y + y);
	}

	mc_object::mc_object(unsigned int init_res) :
		abs_object(new texture(init_res, init_res), 4), 
		m_tex_dim(0)
	{}

	mc_object::~mc_object()
	{}

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
		GLEZ_INFO("mc texture dim: {}", m_tex_dim);

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

}
