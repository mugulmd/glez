#include "camera.h"
#include "log.h"

#include <glm/gtc/matrix_transform.hpp>

namespace glez {

	void camera::update_view()
	{
		m_view = glm::lookAt(m_position, m_target, m_up);
	}

	void camera::update_proj()
	{
		m_proj = glm::perspective(m_fov, m_width / m_height, m_near, m_far);
	}

	glm::vec3 camera::direction()
	{
		return glm::normalize(m_position - m_target);
	}

	glm::vec3 camera::right()
	{
		return glm::normalize(glm::cross(m_up, direction()));
	}

	void camera::set_view_props(glm::vec3 _position, glm::vec3 _target, glm::vec3 _up)
	{
		m_position = _position;
		m_target = _target;
		m_up = _up;
		update_view();
	}

	void camera::rotate_view(float theta_x, float theta_y)
	{
		glm::vec4 view_pos;

		glm::mat4 rot_y = glm::rotate(glm::mat4(1.f), theta_y, -right());
		view_pos = rot_y * glm::vec4(m_position, 1.f);
		if (std::abs(glm::dot(glm::normalize(glm::vec3(view_pos) - m_target), m_up)) > 0.99f) {
			view_pos = glm::vec4(m_position, 1.f);
		}

		glm::mat4 rot_x = glm::rotate(glm::mat4(1.f), theta_x, m_up);
		view_pos = rot_x * view_pos;

		m_position = glm::vec3(view_pos);
		update_view();
	}

	void camera::scale_view(float lambda)
	{
		glm::vec4 view_pos(m_position, 1.f);

		glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(lambda, lambda, lambda));
		view_pos = scale * view_pos;

		m_position = glm::vec3(view_pos);
		update_view();
	}

	void camera::set_proj_props(float w, float h, float fov, float z_near, float z_far)
	{
		m_width = w;
		m_height = h;
		m_fov = fov;
		m_near = z_near;
		m_far = z_far;
		update_proj();
	}

}
