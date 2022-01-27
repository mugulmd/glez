#include "glez_pch.h"

#include "unwrapped_object.h"

namespace glez {

	unwrapped_object::unwrapped_object(quad_mesh* _mesh, texture* _texture) : 
		abs_object(_mesh, _texture, 2)
	{}

	void unwrapped_object::unwrap(std::unordered_map<std::shared_ptr<half_edge>, glm::vec2> tex_coords)
	{
		m_buffer->init_store(m_mesh->get_faces().size());

		for (std::shared_ptr<quad_face>& f : m_mesh->get_faces()) {
			std::vector<float> face_uv(8);
			for (size_t i = 0; i < 4; i++) {
				glm::vec2& uv = tex_coords[f->half_edges[i]];
				face_uv[2 * i] = uv.x;
				face_uv[2 * i + 1] = uv.y;
			}
			m_buffer->store_face(f, face_uv);
		}

		notify_render_buffer_listeners();
	}

}
