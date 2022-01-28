#include "glez_pch.h"

#include "unwrapped_object.h"

namespace glez {

	unwrapped_object::unwrapped_object(texture* _texture) :
		abs_object(_texture, 2)
	{}

	unwrapped_object::~unwrapped_object()
	{}

	void unwrapped_object::add_face(std::shared_ptr<quad_face> face, std::array<glm::vec2, 4> tex_coords)
	{
		m_mesh->add_face(face);

		for (size_t i = 0; i < 4; i++) {
			m_tex_coords[face->half_edges[i]] = tex_coords[i];
		}
	}

	void unwrapped_object::create_uv_layout()
	{
		m_buffer->init_store(m_mesh->get_faces().size());

		for (std::shared_ptr<quad_face>& f : m_mesh->get_faces()) {
			std::vector<float> face_uv(8);
			for (size_t i = 0; i < 4; i++) {
				glm::vec2& uv = m_tex_coords[f->half_edges[i]];
				face_uv[2 * i] = uv.x;
				face_uv[2 * i + 1] = uv.y;
			}
			m_buffer->store_face(f, face_uv);
		}

		notify_render_buffer_listeners();
	}

}
