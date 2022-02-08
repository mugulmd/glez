#include <memory>
#include <array>
#include <vector>
#include <random>
#include <unordered_map>

#include "graffiti_scene.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

graffiti_scene::graffiti_scene() : 
	glez::scene()
{
	/* Mesh Color Object */
	m_obj = new glez::mc_object();
	init_cube();
	m_obj->pack_frames();

	/* Graphics pipeline setup */
	m_renderer = new render_engine();
	m_renderer->init_graphics();
	m_show_selection = true;

	m_obj->create_uv_layout();
	m_renderer->send_to_gpu(m_obj->get_render_buffer());

	glm::u8vec4 base_color(200, 200, 200, 255);
	for (std::shared_ptr<glez::quad_face>& f : m_obj->get_mesh()->get_faces()) {
		m_obj->fill(f, base_color);
	}
	m_renderer->send_to_gpu(m_obj->get_texture());

	/* Selection */
	m_selection = new selection(m_obj);
	m_renderer->send_to_gpu(m_selection);

	/* Painting and Modeling Operations */
	m_painter = nullptr;
	m_displacer = nullptr;
	m_extruder = nullptr;
	m_cutter = nullptr;
}

graffiti_scene::~graffiti_scene()
{
	delete m_obj;
	delete m_renderer;
	delete m_selection;
	if (m_painter) delete m_painter;
	if (m_displacer) delete m_displacer;
	if (m_extruder) delete m_extruder;
	if (m_cutter) delete m_cutter;
}

void graffiti_scene::init_cube()
{
	std::array<glm::vec3, 8> positions{
		glm::vec3(-1, 1, -1), 
		glm::vec3(-1, 1, 1),
		glm::vec3(-1, -1, 1),
		glm::vec3(-1, -1, -1),
		glm::vec3(1, 1, -1),
		glm::vec3(1, 1, 1),
		glm::vec3(1, -1, -1),
		glm::vec3(1, -1, 1)
	};

	std::array<glm::vec3, 6> normals{
		glm::vec3(-1, 0, 0), 
		glm::vec3(0, 1, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(0, 0, 1),
		glm::vec3(0, -1, 0),
		glm::vec3(0, 0, -1)
	};

	std::array<std::array<size_t, 4>, 6> indices{
		std::array<size_t, 4>{0, 1, 2, 3}, 
		std::array<size_t, 4>{0, 4, 5, 1},
		std::array<size_t, 4>{5, 4, 6, 7},
		std::array<size_t, 4>{1, 5, 7, 2},
		std::array<size_t, 4>{2, 7, 6, 3},
		std::array<size_t, 4>{3, 6, 4, 0}
	};

	for (size_t i = 0; i < 8; i++) {
		m_obj->get_mesh()->add_vertex(
			std::make_shared<glez::vertex>(positions[i])
		);
	}

	for (size_t i = 0; i < 6; i++) {
		m_obj->add_face(
			glez::quad_face::make_face(
				{
					m_obj->get_mesh()->get_vertex(indices[i][0]), 
					m_obj->get_mesh()->get_vertex(indices[i][1]),
					m_obj->get_mesh()->get_vertex(indices[i][2]),
					m_obj->get_mesh()->get_vertex(indices[i][3])
				}, 
				normals[i]), 
			128, 128
		);
	}

	m_obj->log_info();
}

void graffiti_scene::display()
{
	m_renderer->render_edges(get_camera());
	if (m_show_selection) {
		m_renderer->render_selection(get_camera());
	}
	m_renderer->render_faces(get_camera());
}

void graffiti_scene::set_selection(const glm::vec2& pick_coords)
{
	glez::ray ray = get_camera()->cast_ray_to(pick_coords);
	std::shared_ptr<glez::quad_face> f = m_obj->get_mesh()->pick_face(ray);
	if (f) {
		m_selection->set(f);
	}
	else {
		m_selection->clear();
	}

	m_selection->update_indices();
	m_renderer->send_to_gpu(m_selection);
}

void graffiti_scene::add_to_selection(const glm::vec2& pick_coords)
{
	glez::ray ray = get_camera()->cast_ray_to(pick_coords);
	std::shared_ptr<glez::quad_face> f = m_obj->get_mesh()->pick_face(ray);
	if (f) {
		m_selection->add(f);
		m_selection->update_indices();
		m_renderer->send_to_gpu(m_selection);
	}
}

void graffiti_scene::select_all()
{
	m_selection->all();
	m_selection->update_indices();
	m_renderer->send_to_gpu(m_selection);
}

void graffiti_scene::invert_selection()
{
	m_selection->inverse();
	m_selection->update_indices();
	m_renderer->send_to_gpu(m_selection);
}

void graffiti_scene::set_painter_fill()
{
	m_painter = new op_fill(m_obj);
	m_show_selection = false;
}

void graffiti_scene::set_painter_spray()
{
	m_painter = new op_spray(m_obj);
	m_show_selection = false;
}

void graffiti_scene::paint(const glm::vec2& pick_coords)
{
	m_painter->apply(get_camera(), pick_coords);
	m_renderer->send_to_gpu(m_obj->get_texture());
}

void graffiti_scene::set_displace_free()
{
	m_displacer = new op_displace_free(m_selection);
	m_show_selection = true;
}

void graffiti_scene::set_displace_x()
{
	m_displacer = new op_displace_axis(m_selection, glm::vec3(1, 0, 0));
	m_show_selection = true;
}

void graffiti_scene::set_displace_y()
{
	m_displacer = new op_displace_axis(m_selection, glm::vec3(0, 1, 0));
	m_show_selection = true;
}

void graffiti_scene::set_displace_z()
{
	m_displacer = new op_displace_axis(m_selection, glm::vec3(0, 0, 1));
	m_show_selection = true;
}

void graffiti_scene::displace(const glm::vec2& pick_start, const glm::vec2& pick_end)
{
	m_displacer->apply(get_camera(), pick_start, pick_end);

	for (const std::shared_ptr<glez::quad_face>& f : m_selection->get_faces()) {
		for (size_t i = 0; i < 4; i++) {
			m_obj->get_render_buffer()->update_vertex(f->half_edges[i]->base);
		}
	}
	m_renderer->send_to_gpu(m_obj->get_render_buffer());
}

void graffiti_scene::set_extrude()
{
	m_extruder = new op_extrude(m_obj);
	m_show_selection = false;
}

void graffiti_scene::init_extrude(const glm::vec2& pick_coords)
{
	m_extruder->init(get_camera(), pick_coords);
	
	m_obj->create_uv_layout();
	m_renderer->send_to_gpu(m_obj->get_render_buffer());

	m_renderer->send_to_gpu(m_obj->get_texture());

	m_selection->update_indices();
	m_renderer->send_to_gpu(m_selection);
}

void graffiti_scene::extrude(const glm::vec2& pick_start, const glm::vec2& pick_end)
{
	m_extruder->apply(get_camera(), pick_start, pick_end);
	
	std::shared_ptr<glez::quad_face> f = m_extruder->get_face();
	if (f) {
		for (size_t i = 0; i < 4; i++) {
			m_obj->get_render_buffer()->update_vertex(f->half_edges[i]->base);
		}
		m_renderer->send_to_gpu(m_obj->get_render_buffer());
	}
}

void graffiti_scene::set_cut()
{
	m_cutter = new op_cut(m_obj);
	m_show_selection = false;
}

void graffiti_scene::cut(const glm::vec2& pick_coords)
{
	m_cutter->apply(get_camera(), pick_coords);
	
	m_obj->create_uv_layout();
	m_renderer->send_to_gpu(m_obj->get_render_buffer());

	m_renderer->send_to_gpu(m_obj->get_texture());

	m_selection->clear();
	m_selection->update_indices();
	m_renderer->send_to_gpu(m_selection);
}
