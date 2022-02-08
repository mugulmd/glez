#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include <vector>

#include "render_engine.h"

#include <glm/gtc/type_ptr.hpp>

bool read_file(const char* filePath, std::string& fileText);
GLuint load_shader(const char* shaderPath, GLenum shaderType);
GLuint make_shader_program(std::vector<GLuint> shaderIDs);

render_engine::render_engine()
{
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ebo_faces);
	glGenBuffers(1, &m_ebo_edges);
	glGenBuffers(1, &m_ebo_selection);
	glGenVertexArrays(1, &m_vao_faces);
	glGenVertexArrays(1, &m_vao_edges);
	glGenVertexArrays(1, &m_vao_selection);
	glGenTextures(1, &m_gltex);

	GLuint vert_bland = load_shader("shaders/bland.vert", GL_VERTEX_SHADER);
	GLuint frag_bland = load_shader("shaders/bland.frag", GL_FRAGMENT_SHADER);
	m_shader_bland = make_shader_program({ vert_bland, frag_bland });

	GLuint vert_mc = load_shader("shaders/mesh_color.vert", GL_VERTEX_SHADER);
	GLuint frag_mc = load_shader("shaders/mesh_color.frag", GL_FRAGMENT_SHADER);
	m_shader_mc = make_shader_program({ vert_mc, frag_mc });

	m_size_faces = 0;
	m_size_edges = 0;
	m_size_selection = 0;
}

void render_engine::init_graphics()
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

	glBindVertexArray(m_vao_selection);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_selection);
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

void render_engine::send_to_gpu(glez::render_buffer* buf)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		buf->vertex_attributes().size() * sizeof(float),
		buf->vertex_attributes().data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_faces);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		buf->face_indices().size() * sizeof(unsigned int),
		buf->face_indices().data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_size_faces = buf->face_indices().size();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_edges);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		buf->edge_indices().size() * sizeof(unsigned int),
		buf->edge_indices().data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_size_edges = buf->edge_indices().size();
}

void render_engine::send_to_gpu(glez::texture* tex)
{
	glBindTexture(GL_TEXTURE_2D, m_gltex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		tex->width(), tex->height(),
		0,
		GL_RGBA, GL_UNSIGNED_BYTE,
		tex->data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void render_engine::send_to_gpu(selection* sel)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_selection);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sel->get_indices().size() * sizeof(unsigned int),
		sel->get_indices().data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_size_selection = sel->get_indices().size();
}

void render_engine::render_faces(glez::camera* cam)
{
	glUseProgram(m_shader_mc);
	GLint viewLoc = glGetUniformLocation(m_shader_mc, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam->get_view()));
	GLint projLoc = glGetUniformLocation(m_shader_mc, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam->get_proj()));

	glBindTexture(GL_TEXTURE_2D, m_gltex);
	glBindVertexArray(m_vao_faces);
	glDrawElements(GL_TRIANGLES,
		m_size_faces,
		GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void render_engine::render_edges(glez::camera* cam)
{
	glUseProgram(m_shader_bland);
	GLint viewLoc = glGetUniformLocation(m_shader_bland, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam->get_view()));
	GLint projLoc = glGetUniformLocation(m_shader_bland, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam->get_proj()));

	glBindVertexArray(m_vao_edges);
	glDrawElements(GL_LINES,
		m_size_edges,
		GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
}

void render_engine::render_selection(glez::camera* cam)
{
	glUseProgram(m_shader_bland);
	GLint viewLoc = glGetUniformLocation(m_shader_bland, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam->get_view()));
	GLint projLoc = glGetUniformLocation(m_shader_bland, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam->get_proj()));

	glBindVertexArray(m_vao_selection);
	glDrawElements(GL_TRIANGLES,
		m_size_selection,
		GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
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
