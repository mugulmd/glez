#pragma once

#include <memory>
#include <array>
#include <list>

#include "core.h"
#include "quad_mesh.h"
#include "texture.h"
#include "render_buffer.h"

namespace glez {

	class GLEZ_API abs_object
	{
	protected:
		quad_mesh* m_mesh;
		texture* m_texture;
		render_buffer* m_buffer;

	public:
		virtual ~abs_object() = 0;
		abs_object(texture* _texture ,unsigned int _uv_dim);

		inline quad_mesh* get_mesh() { return m_mesh; }
		inline texture* get_texture() { return m_texture; }
		inline render_buffer* get_render_buffer() { return m_buffer; }

		void log_info();

	public:
		virtual void create_uv_layout() = 0;

	};

}
