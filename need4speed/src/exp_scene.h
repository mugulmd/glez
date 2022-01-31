#pragma once

#include <memory>
#include <array>

#include "GL/glew.h"

#include "glez.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class exp_scene : public glez::scene
{
private:
	glez::unwrapped_object* m_obj_unwrapped;
	glez::unwrapped_object* m_plane_unwrapped;
	glez::mc_object* m_obj_mc;
	glez::unwrapped_object* m_plane_mc;

private:
	unsigned int res_unwrapped(std::shared_ptr<glez::quad_face> face);
	void transfer_res();
	glm::u8vec4 color_unwrapped(std::shared_ptr<glez::quad_face> face, glm::vec2 coef);
	void transfer_colors();

public:
	exp_scene(const char* obj_file_path, const char* img_file_path);
	~exp_scene();

private:
	glez::abs_object* m_objs[4];
	GLuint m_vboIDs[4];
	GLuint m_eboIDs[4];
	GLuint m_vaoIDs[4];
	GLuint m_texIDs[4];
	GLuint m_shaderIDs[4];

	void init_graphics(size_t idx, unsigned int uv_dim);
	void send_to_gpu(size_t idx, glez::render_buffer* buffer);
	void send_to_gpu(size_t idx, glez::texture* texture);
	void render(size_t idx, glez::abs_object* obj);

public:
	void display();

private:
	unsigned int m_idx_display = 0;
	std::array<float, 1000> m_frame_times;
	int m_n_frames = 0;
	bool m_log_rendering_time = false;

public:
	inline void change_displayed_obj() 
	{ 
		m_idx_display = (m_idx_display + 1) % 4; 
		m_n_frames = 0; 
	}
	inline void log_info_obj() { m_objs[m_idx_display]->log_info(); }
	inline void toggle_log_rendering_time() { m_log_rendering_time = !m_log_rendering_time;  }

};

