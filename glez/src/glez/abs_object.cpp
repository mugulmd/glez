#include "glez_pch.h"

#include "abs_object.h"

namespace glez {

	abs_object::~abs_object() 
	{
		delete m_mesh;
		delete m_texture;
		delete m_buffer;
	}

	abs_object::abs_object(quad_mesh* _mesh, texture* _texture, unsigned int _uv_dim) : 
		m_mesh(_mesh), m_texture(_texture), m_buffer(new render_buffer(_uv_dim))
	{}

	void abs_object::notify_render_buffer_listeners()
	{
		for (render_buffer_listener* listener : m_buffer_listeners) {
			listener->on_render_buffer_change(m_buffer);
		}
	}

	void abs_object::notify_texture_listeners()
	{
		for (texture_listener* listener : m_tex_listeners) {
			listener->on_texture_change(m_texture);
		}
	}

	void abs_object::add_render_buffer_listener(render_buffer_listener* listener)
	{
		m_buffer_listeners.push_back(listener);
	}

	void abs_object::add_texture_listener(texture_listener* listener)
	{
		m_tex_listeners.push_back(listener);
	}

}
