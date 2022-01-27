#pragma once

#include "core.h"

#include <glm/vec4.hpp>

namespace glez {

	class GLEZ_API texture
	{
	private:
		size_t m_width;
		size_t m_height;
		unsigned char* m_data;

	public:
		texture(size_t w, size_t h);
		~texture();

		inline size_t width() { return m_width; }
		inline size_t height() { return m_height; }
		inline unsigned char* data() { return m_data; }

	public:
		void set_pixel(size_t row, size_t col, glm::u8vec4 color);
		void set_pixel(float x, float y, glm::u8vec4 color);

		glm::u8vec4 get_pixel(size_t row, size_t col);
		glm::u8vec4 get_pixel(float x, float y);

	};

	class GLEZ_API texture_listener
	{
	public:
		virtual ~texture_listener() {}

		virtual void on_texture_change(texture* _texture) = 0;

	};

}
