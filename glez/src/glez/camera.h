#pragma once

#include "core.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace glez {

	class GLEZ_API camera
	{
	private:
		glm::vec3 m_position;
		glm::vec3 m_target;
		glm::vec3 m_up;
		glm::mat4 m_view;

		float m_width;
		float m_height;
		float m_fov;
		float m_near;
		float m_far;
		glm::mat4 m_proj;

		void update_view();
		void update_proj();

		glm::vec3 direction();
		glm::vec3 right();

	public:
		void set_view_props(glm::vec3 _position, glm::vec3 _target, glm::vec3 _up);
		void rotate_view(float theta_x, float theta_y);
		void scale_view(float lambda);
		void set_proj_props(float w, float h, float fov = 45.f, float z_near = 0.01f, float z_far = 100.f);

		inline glm::mat4& get_view() { return m_view; }
		inline glm::mat4& get_proj() { return m_proj; }

	};

}
