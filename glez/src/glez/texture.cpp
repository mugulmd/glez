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
		size_t idx = 4 * (row * m_width + col);
		m_data[idx] = color.r;
		m_data[idx + 1] = color.g;
		m_data[idx + 2] = color.b;
		m_data[idx + 3] = color.a;
	}

	void texture::set_pixel(float x, float y, glm::u8vec4 color)
	{
		size_t row = (size_t)(y * (float)(m_height - 1));
		size_t col = (size_t)(x * (float)(m_width - 1));
		set_pixel(row, col, color);
	}

	glm::u8vec4 texture::get_pixel(size_t row, size_t col)
	{
		size_t idx = 4 * (row * m_width + col);
		return glm::u8vec4(m_data[idx], m_data[idx + 1], m_data[idx + 2], m_data[idx + 3]);
	}

	glm::u8vec4 texture::get_pixel(float x, float y)
	{
		size_t row = (size_t)(y * (float)(m_height - 1));
		size_t col = (size_t)(x * (float)(m_width - 1));
		return get_pixel(row, col);
	}

}
