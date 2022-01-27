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
		quad_mesh* m_mesh = nullptr;
		texture* m_texture = nullptr;
		render_buffer* m_buffer;

	public:
		virtual ~abs_object() = 0;
		explicit abs_object(unsigned int _uv_dim) : 
			m_buffer(new render_buffer(_uv_dim))
		{}

		void set_mesh(quad_mesh* _mesh);
		void set_texture(texture* _texture);

		inline texture* get_texture() { return m_texture; }
		inline render_buffer* get_render_buffer() { return m_buffer; }

	protected:
		std::list<render_buffer_listener*> m_buffer_listeners;
		std::list<texture_listener*> m_tex_listeners;

		void notify_render_buffer_listeners();
		void notify_texture_listeners();

	public:
		void add_render_buffer_listener(render_buffer_listener *listener);
		void add_texture_listener(texture_listener *listener);

	};

}
