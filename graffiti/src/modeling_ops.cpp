#include <array>
#include <unordered_map>

#include "modeling_ops.h"

op_displace::op_displace(selection* sel)
{
	for (const std::shared_ptr<glez::quad_face>& f : sel->get_faces()) {
		for (size_t i = 0; i < 4; i++) {
			m_vertices.insert(f->half_edges[i]->base);
		}
	}
}

op_displace_free::op_displace_free(selection* sel) : 
	op_displace(sel)
{}

void op_displace_free::apply(glez::camera* cam, const glm::vec2& pick_start, const glm::vec2& pick_end)
{
	glm::vec3 tr = cam->to_world(pick_end) - cam->to_world(pick_start);

	for (const std::shared_ptr<glez::vertex>& v : m_vertices) {
		v->position += tr;
	}
}

op_displace_axis::op_displace_axis(selection* sel, glm::vec3 axis) : 
	op_displace(sel), 
	m_axis(axis)
{}

void op_displace_axis::apply(glez::camera* cam, const glm::vec2& pick_start, const glm::vec2& pick_end)
{
	glm::vec3 delta = cam->to_world(pick_end) - cam->to_world(pick_start);
	glm::vec3 tr = m_axis * glm::dot(delta, m_axis);

	for (const std::shared_ptr<glez::vertex>& v : m_vertices) {
		v->position += tr;
	}
}

op_extrude::op_extrude(glez::mc_object* obj) :
	m_obj(obj)
{}

void op_extrude::init(glez::camera* cam, const glm::vec2& pick_coords)
{
	glez::ray ray = cam->cast_ray_to(pick_coords);
	m_face = m_obj->get_mesh()->pick_face(ray);
	if (!m_face) return;

	// store old frames before adding new faces
	std::unordered_map<std::shared_ptr<glez::quad_face>, glez::frame> old_frames;
	for (std::shared_ptr<glez::quad_face>& f : m_obj->get_mesh()->get_faces()) {
		old_frames[f] = m_obj->get_frame(f);
	}

	// create new vertices, normals and faces and (un)connect them
	std::array<std::shared_ptr<glez::vertex>, 4> loop_vertices;
	std::array<glm::vec3, 4> loop_normals;
	std::array<std::shared_ptr<glez::quad_face>, 4> loop_faces;

	for (size_t i = 0; i < 4; i++) {
		std::shared_ptr<glez::half_edge> h = m_face->half_edges[i];

		glm::vec3 p = h->base->position;
		loop_vertices[i] = std::make_shared<glez::vertex>(p);
		m_obj->get_mesh()->add_vertex(loop_vertices[i]);

		loop_normals[i] = glm::normalize(
			glm::cross(
				h->next->base->position - h->base->position,
				h->face->normal
			)
		);
	}

	for (size_t i = 0; i < 4; i++) {
		size_t j = (i + 1) % 4;
		std::shared_ptr<glez::quad_face> f = glez::quad_face::make_face(
			{
				m_face->half_edges[i]->base,
				m_face->half_edges[j]->base,
				loop_vertices[j],
				loop_vertices[i]
			},
			loop_normals[i]
		);
		loop_faces[i] = f;
		m_obj->add_face(f, 128, 128);
	}

	for (size_t i = 0; i < 4; i++) {
		std::shared_ptr<glez::vertex> v = loop_faces[i]->half_edges[0]->base;
		std::shared_ptr<glez::half_edge> h = m_face->half_edges[i];
		v->half_edges.remove(h);
		h->base = loop_vertices[i];
	}

	for (size_t i = 0; i < 4; i++) {
		size_t i_prev = (i - 1) % 4;
		size_t i_next = (i + 1) % 4;

		std::shared_ptr<glez::half_edge> h = m_face->half_edges[i];
		std::shared_ptr<glez::half_edge> h_out = h->opposite;

		loop_faces[i]->half_edges[0]->opposite = h_out;
		h_out->opposite = loop_faces[i]->half_edges[0];

		loop_faces[i]->half_edges[1]->opposite = loop_faces[i_next]->half_edges[3];

		loop_faces[i]->half_edges[2]->opposite = h;
		h->opposite = loop_faces[i]->half_edges[2];

		loop_faces[i]->half_edges[3]->opposite = loop_faces[i_prev]->half_edges[1];
	}

	for (size_t i = 0; i < 4; i++) {
		loop_vertices[i]->half_edges.push_back(m_face->half_edges[i]);

		size_t i_prev = (i - 1) % 4;
		loop_vertices[i]->half_edges.push_back(m_face->half_edges[i_prev]->next);
	}

	// copy old texture
	glez::texture* old_texture = new glez::texture(m_obj->get_texture()->width(), m_obj->get_texture()->height());
	for (unsigned int x = 0; x < old_texture->width(); x++) {
		for (unsigned int y = 0; y < old_texture->height(); y++) {
			old_texture->set_pixel(x, y, m_obj->get_texture()->get_pixel(x, y));
		}
	}

	// pack new frames
	m_obj->pack_frames();

	// transfer texture
	for (auto entry : old_frames) {
		glez::frame fr_old = entry.second;
		glez::frame& fr_new = m_obj->get_frame(entry.first);
		for (unsigned int x = 0; x <= fr_new.res.x; x++) {
			for (unsigned int y = 0; y <= fr_new.res.y; y++) {
				fr_new.set_pixel(m_obj->get_texture(), x, y, fr_old.get_pixel(old_texture, x, y));
			}
		}
	}

	// init texture on new faces
	glm::u8vec4 base_color(200, 200, 200, 255);
	for (size_t i = 0; i < 4; i++) {
		std::shared_ptr<glez::quad_face> f = loop_faces[i];
		m_obj->fill(f, base_color);
	}
}

void op_extrude::apply(glez::camera* cam, const glm::vec2& pick_start, const glm::vec2& pick_end)
{
	if (!m_face) return;

	glm::vec3 delta = cam->to_world(pick_end) - cam->to_world(pick_start);
	float offset = glm::dot(delta, m_face->normal);

	for (size_t i = 0; i < 4; i++) {
		m_face->half_edges[i]->base->position += m_face->normal * offset;
	}
}

op_cut::op_cut(glez::mc_object* obj) : 
	m_obj(obj)
{}

void op_cut::apply(glez::camera* cam, const glm::vec2& pick_coords)
{
	// TODO
}
