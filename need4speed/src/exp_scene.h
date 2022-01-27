#pragma once

#include "GL/glew.h"

#include "glez.h"

class exp_scene : public glez::scene, 
	public glez::render_buffer_listener, 
	public glez::texture_listener
{
private:
	glez::unwrapped_object* m_obj;

	GLuint m_vbo;
	GLuint m_ebo;
	GLuint m_vao;
	GLuint m_shader;

public:
	exp_scene();
	~exp_scene();

	void on_render_buffer_change(glez::render_buffer* buffer) override;
	void on_texture_change(glez::texture* texture) override;

	void display();

};

