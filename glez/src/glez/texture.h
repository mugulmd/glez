#pragma once

#include "core.h"

#include <glm/vec4.hpp>

namespace glez {

	class GLEZ_API texture
	{
	private:
		unsigned int m_width;
		unsigned int m_height;
		unsigned char* m_data;

	public:
		texture(unsigned int w, unsigned int h);
		texture(unsigned int w, unsigned int h, unsigned char* _data);
		~texture();

		inline unsigned int width() { return m_width; }
		inline unsigned int height() { return m_height; }
		inline unsigned char* data() { return m_data; }

	public:
		void set_pixel(unsigned int row, unsigned int col, const glm::u8vec4& color);
		void set_pixel(float x, float y, const glm::u8vec4& color);

		glm::u8vec4 get_pixel(unsigned int row, unsigned int col);
		glm::u8vec4 get_pixel(float x, float y);

	};

	class GLEZ_API texture_listener
	{
	public:
		virtual ~texture_listener() {}

		virtual void on_texture_change(texture* _texture) = 0;

	};

}
