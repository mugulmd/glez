#pragma once

#include "core.h"

namespace glez {

	class GLEZ_API texture
	{
	public:
		// TODO

	};

	class GLEZ_API texture_listener
	{
	public:
		virtual ~texture_listener() {}

		virtual void on_texture_change(const texture& _texture) = 0;

	};

}
