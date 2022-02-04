#include <memory>
#include <array>
#include <vector>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "graffiti_scene.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

bool read_file(const char* filePath, std::string& fileText);
GLuint load_shader(const char* shaderPath, GLenum shaderType);
GLuint make_shader_program(std::vector<GLuint> shaderIDs);

graffiti_scene::graffiti_scene() : 
	glez::scene()
{
	/* Graphics pipeline setup */
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ebo_faces);
	glGenBuffers(1, &m_ebo_edges);
	glGenVertexArrays(1, &m_vao_faces);
	glGenVertexArrays(1, &m_vao_edges);
	glGenTextures(1, &m_gltex);

	init_graphics();

	/* Shaders */
	GLuint vert_bland = load_shader("shaders/bland.vert", GL_VERTEX_SHADER);
	GLuint frag_bland = load_shader("shaders/bland.frag", GL_FRAGMENT_SHADER);
	m_shader_bland = make_shader_program({ vert_bland, frag_bland });

	GLuint vert_mc = load_shader("shaders/mesh_color.vert", GL_VERTEX_SHADER);
	GLuint frag_mc = load_shader("shaders/mesh_color.frag", GL_FRAGMENT_SHADER);
	m_shader_mc = make_shader_program({ vert_mc, frag_mc });

	/* Mesh Color Object */
	m_obj = new glez::mc_object();

	init_cube();
	m_obj->pack_frames();
	m_obj->create_uv_layout();
	send_to_gpu(m_obj->get_render_buffer());

	glm::u8vec4 base_color(200, 200, 200, 255);
	for (std::shared_ptr<glez::quad_face>& f : m_obj->get_mesh()->get_faces()) {
		m_obj->fill(f, base_color);
	}
	send_to_gpu(m_obj->get_texture());
}

graffiti_scene::~graffiti_scene()
{
	delete m_obj;
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

void graffiti_scene::init_graphics()
{
	glBindVertexArray(m_vao_faces);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_faces);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coordinates
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (GLvoid*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	glBindVertexArray(m_vao_edges);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_edges);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, m_gltex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void graffiti_scene::send_to_gpu(glez::render_buffer* buffer)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		buffer->vertex_attributes().size() * sizeof(float),
		buffer->vertex_attributes().data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_faces);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		buffer->face_indices().size() * sizeof(unsigned int),
		buffer->face_indices().data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_edges);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		buffer->edge_indices().size() * sizeof(unsigned int),
		buffer->edge_indices().data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void graffiti_scene::send_to_gpu(glez::texture* texture)
{
	glBindTexture(GL_TEXTURE_2D, m_gltex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		texture->width(), texture->height(),
		0,
		GL_RGBA, GL_UNSIGNED_BYTE,
		texture->data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void graffiti_scene::render_faces()
{
	glUseProgram(m_shader_mc);
	GLint viewLoc = glGetUniformLocation(m_shader_mc, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(get_camera()->get_view()));
	GLint projLoc = glGetUniformLocation(m_shader_mc, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(get_camera()->get_proj()));

	glBindTexture(GL_TEXTURE_2D, m_gltex);
	glBindVertexArray(m_vao_faces);
	glDrawElements(GL_TRIANGLES,
		m_obj->get_render_buffer()->face_indices().size(),
		GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void graffiti_scene::render_edges()
{
	glUseProgram(m_shader_bland);
	GLint viewLoc = glGetUniformLocation(m_shader_bland, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(get_camera()->get_view()));
	GLint projLoc = glGetUniformLocation(m_shader_bland, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(get_camera()->get_proj()));

	glBindVertexArray(m_vao_edges);
	glDrawElements(GL_LINES,
		m_obj->get_render_buffer()->edge_indices().size(),
		GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
}

void graffiti_scene::display()
{
	render_faces();
	render_edges();
}

/* Utils */

bool read_file(const char* filePath, std::string& fileText)
{
	try {
		std::ifstream file;
		file.open(filePath, std::ios::in);
		std::stringstream fileStream;
		fileStream << file.rdbuf();
		file.close();
		fileText = fileStream.str();
		return true;
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::FILE_NOT_SUCCESSFULLY_READ : " << filePath << std::endl;
		return false;
	}
}

GLuint load_shader(const char* shaderPath, GLenum shaderType)
{
	// Retrieve source code from file
	std::string code;
	read_file(shaderPath, code);
	const GLchar* shaderCode = code.c_str();

	// Compile
	GLuint shaderId;
	shaderId = glCreateShader(shaderType);
	glShaderSource(shaderId, 1, &shaderCode, NULL);
	glCompileShader(shaderId);

	// Check for errors
	GLint success;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
		return 0;
	}

	return shaderId;
}

GLuint make_shader_program(std::vector<GLuint> shaderIDs)
{
	GLuint program_ID = glCreateProgram();
	for (GLuint shaderID : shaderIDs) {
		glAttachShader(program_ID, shaderID);
	}
	glLinkProgram(program_ID);

	GLint success;
	glGetProgramiv(program_ID, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetProgramInfoLog(program_ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	for (GLuint shaderID : shaderIDs) {
		glDeleteShader(shaderID);
	}

	return program_ID;
}

