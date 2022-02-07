#pragma once

#include <memory>
#include <vector>
#include <array>
#include <unordered_map>

#include "core.h"
#include "quad_mesh.h"

namespace glez {

	class GLEZ_API render_buffer
	{
	private:
		unsigned int m_uv_dim;

	public:
		explicit render_buffer(unsigned int _uv_dim);

	private:
		std::vector<float> m_vertex_attrib;
		std::vector<unsigned int> m_face_indices;
		std::vector<unsigned int> m_edge_indices;

	public:
		inline unsigned int uv_dim() { return m_uv_dim; }
		inline std::vector<float>& vertex_attributes() { return m_vertex_attrib; }
		inline std::vector<unsigned int>& face_indices() { return m_face_indices; }
		inline std::vector<unsigned int>& edge_indices() { return m_edge_indices; }

	private:
		unsigned int m_face_counter;
		std::unordered_map<std::shared_ptr<quad_face>, unsigned int> m_counters;

	public:
		void init_store(size_t n_faces);
		void store_face(std::shared_ptr<quad_face> face, std::vector<float> tex_coords);

	public:
		template<class Iterator>
		std::vector<unsigned int> sub_face_indices(const Iterator& first, const Iterator& last)
		{
			std::vector<unsigned int> indices;

			std::array<size_t, 6> triangles{ 0, 1, 2, 2, 3, 0 };
			for (Iterator it = first; it != last; it++) {
				for (size_t i = 0; i < 6; i++) {
					size_t v_idx = m_counters[*it] * 4 + triangles[i];
					indices.push_back(v_idx);
				}
			}

			return indices;
		}
	};

}
