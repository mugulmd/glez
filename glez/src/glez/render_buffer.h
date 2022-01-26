#pragma once

#include <memory>
#include <vector>

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

	public:
		void init_store(size_t n_faces);
		void store_face(std::shared_ptr<quad_face> face, std::vector<float> tex_coords);

	};

	class GLEZ_API render_buffer_listener
	{
	public:
		virtual ~render_buffer_listener() {}

		virtual void on_render_buffer_change(const render_buffer& _buffer) = 0;

	};

}
