#pragma once

#include "GL/glew.h"

#include "glez.h"
#include "selection.h"

class render_engine
{
private:
	GLuint m_vbo;
	GLuint m_vao_faces, m_vao_edges, m_vao_selection;
	GLuint m_ebo_faces, m_ebo_edges, m_ebo_selection;
	GLuint m_gltex;
	GLuint m_shader_bland, m_shader_mc;

	size_t m_size_faces, m_size_edges, m_size_selection;

public:
	render_engine();

	void init_graphics();
	void send_to_gpu(glez::render_buffer* buf);
	void send_to_gpu(glez::texture* tex);
	void send_to_gpu(selection* sel);
	void render_faces(glez::camera* cam);
	void render_edges(glez::camera* cam);
	void render_selection(glez::camera* cam);

};
