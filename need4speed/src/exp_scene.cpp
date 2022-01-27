#include <memory>
#include <unordered_map>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "exp_scene.h"

#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>

bool read_file(const char* filePath, std::string& fileText);
GLuint load_shader(const char* shaderPath, GLenum shaderType);
GLuint make_shader_program(std::vector<GLuint> shaderIDs);

exp_scene::exp_scene()
{
	glez::quad_mesh* plane = glez::quad_mesh::make_plane();
	glez::texture* texture = new glez::texture(); // TODO : read from file
	m_obj = new glez::unwrapped_object(plane, texture);
	m_obj->add_render_buffer_listener(this);
	m_obj->add_texture_listener(this);

	unsigned int uv_dim = m_obj->get_render_buffer()->uv_dim();
	unsigned int stride = 6 + uv_dim;

	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ebo);
	glGenVertexArrays(1, &m_vao);

	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coordinates
	glVertexAttribPointer(2, uv_dim, GL_FLOAT, GL_FALSE, stride * sizeof(float), (GLvoid*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

	glBindVertexArray(0);

	std::unordered_map<std::shared_ptr<glez::half_edge>, glm::vec2> tex_coords;
	tex_coords[plane->get_faces().front()->half_edges[0]] = glm::vec2(0, 1);
	tex_coords[plane->get_faces().front()->half_edges[1]] = glm::vec2(1, 1);
	tex_coords[plane->get_faces().front()->half_edges[2]] = glm::vec2(1, 0);
	tex_coords[plane->get_faces().front()->half_edges[3]] = glm::vec2(0, 0);
	m_obj->unwrap(tex_coords);

	GLuint vert_bland = load_shader("shaders/bland.vert", GL_VERTEX_SHADER);
	GLuint frag_bland = load_shader("shaders/bland.frag", GL_FRAGMENT_SHADER);
	m_shader = make_shader_program({vert_bland, frag_bland});
}

exp_scene::~exp_scene()
{
	delete m_obj;
}

void exp_scene::on_render_buffer_change(glez::render_buffer* buffer)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, 
		m_obj->get_render_buffer()->vertex_attributes().size() * sizeof(float), 
		m_obj->get_render_buffer()->vertex_attributes().data(), 
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		m_obj->get_render_buffer()->face_indices().size() * sizeof(unsigned int),
		m_obj->get_render_buffer()->face_indices().data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void exp_scene::on_texture_change(glez::texture* texture)
{
	// TODO
}

void exp_scene::display()
{
	glUseProgram(m_shader);

	GLint viewLoc = glGetUniformLocation(m_shader, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(get_camera()->get_view()));
	GLint projLoc = glGetUniformLocation(m_shader, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(get_camera()->get_proj()));

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, 
		m_obj->get_render_buffer()->face_indices().size(), 
		GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
}

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
