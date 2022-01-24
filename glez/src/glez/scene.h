#pragma once

#include <memory>

#include "core.h"
#include "camera.h"

namespace glez {

	class GLEZ_API scene 
	{
	private:
		std::unique_ptr<camera> m_camera;

	public:
		scene();

	};

}
