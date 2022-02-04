#pragma once

#include "GL/glew.h"

#include "glez.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class graffiti_scene : public glez::scene
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
	GLuint m_vao_faces, m_vao_edges;
	GLuint m_ebo_faces, m_ebo_edges;
	GLuint m_gltex;
	GLuint m_shader_bland, m_shader_mc;

	void init_graphics();
	void send_to_gpu(glez::render_buffer* buffer);
	void send_to_gpu(glez::texture* texture);
	void render_faces();
	void render_edges();

public:
	void display();

private:
	glm::u8vec4 m_color = glm::u8vec4(255, 0, 0, 255);

public:
	inline void set_color(glm::u8vec4 color) { m_color = color; }
	void fill(const glm::vec2& pick_coords);
	void spraypaint(const glm::vec2& pick_coords);

};
