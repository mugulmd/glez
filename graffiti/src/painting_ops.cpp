#include <random>

#include "painting_ops.h"

#include <glm/vec4.hpp>

float op_paint::color[4] = {1.f, 0.f, 0.f, 1.f};

op_paint::op_paint(glez::mc_object* obj) : 
	m_obj(obj)
{}

op_fill::op_fill(glez::mc_object* obj) : 
	op_paint(obj)
{}

void op_fill::apply(glez::camera* cam, const glm::vec2& pick_coords)
{
	glez::ray ray = cam->cast_ray_to(pick_coords);
	std::shared_ptr<glez::quad_face> f = m_obj->get_mesh()->pick_face(ray);
	if (f) {
		m_obj->fill(f, glm::u8vec4(255 * color[0], 255 * color[1], 255 * color[2], 255 * color[3]));
	}
}

float op_spray::radius = 0.01f;
int op_spray::n_ray = 10;

op_spray::op_spray(glez::mc_object* obj) : 
	op_paint(obj)
{}

void op_spray::apply(glez::camera* cam, const glm::vec2& pick_coords)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(-1.f, 1.f);

	for (unsigned int iter = 0; iter < n_ray; iter++) {
		float dist;
		glm::vec2 coords;
		float theta = dis(gen) * 3.14f;
		float r = dis(gen) * radius;
		glm::vec2 offset(std::cos(theta) * r, std::sin(theta) * r);
		glez::ray ray = cam->cast_ray_to(pick_coords + offset);
		std::shared_ptr<glez::quad_face> f = m_obj->get_mesh()->pick_face(ray, dist, coords);
		if (f) {
			m_obj->paint(f, coords, glm::u8vec4(255 * color[0], 255 * color[1], 255 * color[2], 255 * color[3]));
		}
	}
}
