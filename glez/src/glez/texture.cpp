#include "glez_pch.h"

#include "texture.h"

namespace glez {

	texture::texture(size_t w, size_t h) : 
		m_width(w), m_height(h)
	{
		size_t length = w * h * 4;
		m_data = new unsigned char[length];
	}

	texture::~texture()
	{
		delete m_data;
	}

	void texture::set_pixel(size_t row, size_t col, glm::u8vec4 color)
	{
		m_data[4 * (row * m_width + col)] = color.r;
		m_data[4 * (row * m_width + col) + 1] = color.g;
		m_data[4 * (row * m_width + col) + 2] = color.b;
		m_data[4 * (row * m_width + col) + 3] = color.a;
	}

}
