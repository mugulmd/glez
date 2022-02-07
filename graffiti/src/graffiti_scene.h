#pragma once

#include "GL/glew.h"

#include "glez.h"
#include "selection.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class graffiti_scene : 
	public glez::scene
{
private:
	glez::mc_object* m_obj;

	void init_cube();

public:
	graffiti_scene();
	~graffiti_scene();

	inline void log_object_info() { m_obj->log_info(); }

private:
	GLuint m_vbo;
	GLuint m_vao_faces, m_vao_edges, m_vao_selection;
	GLuint m_ebo_faces, m_ebo_edges, m_ebo_selection;
	GLuint m_gltex;
	GLuint m_shader_bland, m_shader_mc;

	void init_graphics();
	void send_to_gpu(glez::render_buffer* buffer);
	void send_to_gpu(glez::texture* texture);
	void send_to_gpu(selection* _selection);
	void render_faces();
	void render_edges();
	void render_selection();

public:
	void display();

public:
	void fill(const glm::vec2& pick_coords, const glm::u8vec4& color);
	void spraypaint(const glm::vec2& pick_coords, const glm::u8vec4& color, const float& radius);

private:
	selection* m_selection;

public:
	inline selection* get_selection() { return m_selection; }
	void set_selection(const glm::vec2& pick_coords);
	void add_to_selection(const glm::vec2& pick_coords);
	void select_all();
	void invert_selection();

};
