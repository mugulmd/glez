#include "glez_pch.h"

#include "render_buffer.h"

namespace glez {

	render_buffer::render_buffer(unsigned int _uv_dim) : 
		m_uv_dim(_uv_dim)
	{}

	void render_buffer::init_store(size_t n_faces)
	{
		m_vertex_attrib = std::vector<float>(n_faces * 4 * (6 + uv_dim()));
		m_face_indices = std::vector<unsigned int>(n_faces * 6);
		m_edge_indices = std::vector<unsigned int>(n_faces * 8);
		m_face_counter = 0;
		m_counters.clear();
	}

	void render_buffer::store_face(std::shared_ptr<quad_face> face, std::vector<float> tex_coords)
	{
		glm::vec3 n = face->normal;

		for (size_t i = 0; i < 4; i++) {
			glm::vec3& p = face->half_edges[i]->base->position;
			size_t idx = (m_face_counter * 4 + i) * (6 + uv_dim());
			m_vertex_attrib[idx] = p.x; idx++;
			m_vertex_attrib[idx] = p.y; idx++;
			m_vertex_attrib[idx] = p.z; idx++;
			m_vertex_attrib[idx] = n.x; idx++;
			m_vertex_attrib[idx] = n.y; idx++;
			m_vertex_attrib[idx] = n.z; idx++;
			for (size_t j = 0; j < uv_dim(); j++) {
				m_vertex_attrib[idx] = tex_coords[i * uv_dim() + j]; idx++;
			}
		}

		std::array<size_t, 6> triangles{0, 1, 2, 2, 3, 0};
		for (size_t i = 0; i < 6; i++) {
			size_t idx = m_face_counter * 6 + i;
			size_t v_idx = m_face_counter * 4 + triangles[i];
			m_face_indices[idx] = v_idx;
		}

		std::array<size_t, 8> lines{0, 1, 1, 2, 2, 3, 3, 0};
		for (size_t i = 0; i < 8; i++) {
			size_t idx = m_face_counter * 8 + i;
			size_t v_idx = m_face_counter * 4 + lines[i];
			m_edge_indices[idx] = v_idx;
		}

		m_counters[face] = m_face_counter;
		m_face_counter++;
	}

}
