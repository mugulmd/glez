#include "glez_pch.h"

#include "abs_object.h"

namespace glez {

	abs_object::abs_object(texture* _texture, unsigned int _uv_dim) :
		m_texture(_texture), m_mesh(new quad_mesh()), m_buffer(new render_buffer(_uv_dim))
	{}

	abs_object::~abs_object() 
	{
		delete m_mesh;
		delete m_texture;
		delete m_buffer;
	}

	void abs_object::log_info()
	{
		GLEZ_INFO(
			"object info: n_vertices={}, n_faces={}, tex_w={}, tex_h={}", 
			m_mesh->get_vertices().size(), 
			m_mesh->get_faces().size(), 
			m_texture->width(), 
			m_texture->height()
		);
	}

}
