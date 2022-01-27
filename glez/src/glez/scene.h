#pragma once

#include <memory>

#include "core.h"
#include "camera.h"

namespace glez {

	class GLEZ_API scene 
	{
	private:
		camera* m_camera;

	public:
		scene();
		virtual ~scene();

		inline camera* get_camera() { return m_camera; }

	};

}
