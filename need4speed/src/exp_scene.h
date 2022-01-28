#pragma once

#include "GL/glew.h"

#include "glez.h"

class exp_scene : public glez::scene
{
private:
	glez::unwrapped_object* m_obj;

	GLuint m_vboIDs[2];
	GLuint m_eboIDs[2];
	GLuint m_vaoIDs[2];
	GLuint m_texIDs[2];
	GLuint m_shaderIDs[2];

	void init_graphics(size_t idx, unsigned int uv_dim);
	void send_to_gpu(size_t idx, glez::render_buffer* buffer);
	void send_to_gpu(size_t idx, glez::texture* texture);
	void render(size_t idx);

public:
	exp_scene();
	~exp_scene();

	void display();

};

