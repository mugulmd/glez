#include "glez_pch.h"

#include "texture.h"

namespace glez {

	texture::texture(unsigned int w, unsigned int h) :
		m_width(w), m_height(h)
	{
		size_t length = w * h * 4;
		m_data = new unsigned char[length];
		for (size_t i = 0; i < length; i++) {
			m_data[i] = 0;
		}
	}

	texture::texture(unsigned int w, unsigned int h, unsigned char* _data) :
		m_width(w), m_height(h), m_data(_data)
	{}

	texture::~texture()
	{
		delete m_data;
	}

	void texture::set_pixel(unsigned int row, unsigned int col, const glm::u8vec4& color)
	{
		size_t idx = 4 * (row * m_width + col);
		m_data[idx] = color.r;
		m_data[idx + 1] = color.g;
		m_data[idx + 2] = color.b;
		m_data[idx + 3] = color.a;
	}

	void texture::set_pixel(float x, float y, const glm::u8vec4& color)
	{
		unsigned int row = (unsigned int)(y * (float)(m_height - 1));
		unsigned int col = (unsigned int)(x * (float)(m_width - 1));
		set_pixel(row, col, color);
	}

	glm::u8vec4 texture::get_pixel(unsigned int row, unsigned int col)
	{
		unsigned int idx = 4 * (row * m_width + col);
		return glm::u8vec4(m_data[idx], m_data[idx + 1], m_data[idx + 2], m_data[idx + 3]);
	}

	glm::u8vec4 texture::get_pixel(float x, float y)
	{
		unsigned int row = (unsigned int)(y * (float)(m_height - 1));
		unsigned int col = (unsigned int)(x * (float)(m_width - 1));
		return get_pixel(row, col);
	}

	mipmap::mipmap(texture* base_tex)
	{
		levels.push_back(base_tex);
	}

	mipmap::~mipmap()
	{
		for (size_t i = 1; i < levels.size(); i++) {
			delete levels[i];
		}
	}

	texture* mipmap::add_level(unsigned int w, unsigned int h)
	{
		texture* level = new texture(w, h);
		levels.push_back(level);
		return level;
	}

}
