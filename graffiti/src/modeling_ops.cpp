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

	// copy old texture
	glez::texture* old_texture = new glez::texture(m_obj->get_texture()->width(), m_obj->get_texture()->height());
	for (unsigned int x = 0; x < old_texture->width(); x++) {
		for (unsigned int y = 0; y < old_texture->height(); y++) {
			old_texture->set_pixel(x, y, m_obj->get_texture()->get_pixel(x, y));
		}
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
	// picked half-edge
	glez::ray ray = cam->cast_ray_to(pick_coords);
	std::shared_ptr<glez::half_edge> h = m_obj->get_mesh()->pick_edge(ray);
	if (!h) return;

	// store old frames before adding new faces
	std::unordered_map<std::shared_ptr<glez::quad_face>, glez::frame> old_frames;
	for (std::shared_ptr<glez::quad_face>& f : m_obj->get_mesh()->get_faces()) {
		old_frames[f] = m_obj->get_frame(f);
	}

	// copy old texture
	glez::texture* old_texture = new glez::texture(m_obj->get_texture()->width(), m_obj->get_texture()->height());
	for (unsigned int x = 0; x < old_texture->width(); x++) {
		for (unsigned int y = 0; y < old_texture->height(); y++) {
			old_texture->set_pixel(x, y, m_obj->get_texture()->get_pixel(x, y));
		}
	}

	// extract loop
	std::vector<std::shared_ptr<glez::half_edge>> loop_half_edges;
	std::vector<std::shared_ptr<glez::quad_face>> loop_faces;

	loop_half_edges.push_back(h);
	loop_half_edges.push_back(h->next->next);
	loop_faces.push_back(h->face);
	while (loop_half_edges.front()->opposite != loop_half_edges.back()) {
		loop_half_edges.push_back(loop_half_edges.back()->opposite);
		loop_half_edges.push_back(loop_half_edges.back()->next->next);
		loop_faces.push_back(loop_half_edges.back()->face);
	}

	size_t n = loop_faces.size();

	// remove old faces
	for (std::shared_ptr<glez::quad_face>& f : loop_faces) {
		m_obj->remove_face(f);
	}

	// create and add new vertices and faces
	std::vector<std::shared_ptr<glez::vertex>> new_vertices;
	std::vector<std::shared_ptr<glez::quad_face>> new_faces;

	for (size_t i = 0; i < n; i++) {
		new_vertices.push_back(std::make_shared<glez::vertex>(loop_half_edges[2 * i]->mid_point()));
		m_obj->get_mesh()->add_vertex(new_vertices[i]);
	}

	for (size_t i = 0; i < n; i++) {
		size_t j = (i + 1) % n;

		new_faces.push_back(glez::quad_face::make_face(
			{
				new_vertices[i], 
				loop_half_edges[2 * i]->next->base, 
				loop_half_edges[2 * i + 1]->base, 
				new_vertices[j]
			}, 
			loop_faces[i]->normal)
		);
		m_obj->add_face(new_faces[2 * i], 128, 128);

		new_faces.push_back(glez::quad_face::make_face(
			{
				loop_half_edges[2 * i]->base, 
				new_vertices[i], 
				new_vertices[j], 
				loop_half_edges[2 * i + 1]->next->base
			},
			loop_faces[i]->normal)
		);
		m_obj->add_face(new_faces[2 * i + 1], 128, 128);
	}

	// connect new faces
	for (size_t i = 0; i < n; i++) {
		size_t i_prev = (i - 1) % n;
		size_t i_next = (i + 1) % n;

		std::shared_ptr<glez::half_edge> h_left = loop_half_edges[2 * i]->next->opposite;
		std::shared_ptr<glez::half_edge> h_right = loop_half_edges[2 * i + 1]->next->opposite;

		new_faces[2 * i]->half_edges[0]->opposite = new_faces[2 * i_prev]->half_edges[2];
		new_faces[2 * i]->half_edges[1]->opposite = h_left;
		h_left->opposite = new_faces[2 * i]->half_edges[1];
		new_faces[2 * i]->half_edges[2]->opposite = new_faces[2 * i_next]->half_edges[0];
		new_faces[2 * i]->half_edges[3]->opposite = new_faces[2 * i + 1]->half_edges[1];

		new_faces[2 * i + 1]->half_edges[0]->opposite = new_faces[2 * i_prev + 1]->half_edges[2];
		new_faces[2 * i + 1]->half_edges[1]->opposite = new_faces[2 * i]->half_edges[3];
		new_faces[2 * i + 1]->half_edges[2]->opposite = new_faces[2 * i_next + 1]->half_edges[0];
		new_faces[2 * i + 1]->half_edges[3]->opposite = h_right;
		h_right->opposite = new_faces[2 * i + 1]->half_edges[3];
	}

	// pack new frames
	m_obj->pack_frames();

	// transfer texture
	for (size_t i = 0; i < n; i++) {
		glez::frame fr_old = old_frames[loop_faces[i]];
		size_t idx = loop_half_edges[2 * 1]->local_idx;

		glez::frame& fr_left = m_obj->get_frame(new_faces[2 * i]);
		for (unsigned int x = 0; x <= fr_left.res.x; x++) {
			for (unsigned int y = 0; y <= fr_left.res.y; y++) {
				float x_norm = (float)x / (float)(fr_left.res.x);
				float y_norm = (float)y / (float)(fr_left.res.y);
				float x_map = (idx == 0) ? 0.5f * (1 + x_norm) :
					(idx == 1) ? 1 - y_norm :
					(idx == 2) ? 0.5f * (1 - x_norm) :
					y_norm;
				float y_map = (idx == 0) ? y_norm :
					(idx == 1) ? 0.5f * (1 + x_norm) :
					(idx == 2) ? 1 - y_norm :
					0.5f * (1 - x_norm);
				fr_left.set_pixel(m_obj->get_texture(), x, y, fr_old.get_pixel(old_texture, x_map, y_map));
			}
		}

		glez::frame& fr_right = m_obj->get_frame(new_faces[2 * i + 1]);
		for (unsigned int x = 0; x <= fr_right.res.x; x++) {
			for (unsigned int y = 0; y <= fr_right.res.y; y++) {
				float x_norm = (float)x / (float)(fr_right.res.x);
				float y_norm = (float)y / (float)(fr_right.res.y);
				float x_map = (idx == 0) ? 0.5f * x_norm :
					(idx == 1) ? 1 - y_norm :
					(idx == 2) ? 1 - 0.5f * x_norm :
					y_norm;
				float y_map = (idx == 0) ? y_norm :
					(idx == 1) ? 0.5f * x_norm :
					(idx == 2) ? 1 - y_norm :
					1 - 0.5f * x_norm;
				fr_right.set_pixel(m_obj->get_texture(), x, y, fr_old.get_pixel(old_texture, x_map, y_map));
			}
		}

		old_frames.erase(loop_faces[i]);
	}

	for (auto entry : old_frames) {
		glez::frame fr_old = entry.second;
		glez::frame& fr_new = m_obj->get_frame(entry.first);
		for (unsigned int x = 0; x <= fr_new.res.x; x++) {
			for (unsigned int y = 0; y <= fr_new.res.y; y++) {
				fr_new.set_pixel(m_obj->get_texture(), x, y, fr_old.get_pixel(old_texture, x, y));
			}
		}
	}
}
