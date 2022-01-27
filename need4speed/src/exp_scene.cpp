#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "exp_scene.h"

#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>

bool read_file(const char* filePath, std::string& fileText);
void* load_obj(const char* objFilePath, glez::unwrapped_object* obj);
GLuint load_shader(const char* shaderPath, GLenum shaderType);
GLuint make_shader_program(std::vector<GLuint> shaderIDs);

exp_scene::exp_scene()
{
	m_obj = new glez::unwrapped_object();
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

	load_obj("res/cube.obj", m_obj);

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

void* load_obj(const char* objFilePath, glez::unwrapped_object* obj)
{
	std::ifstream ifs(objFilePath, std::ios::in);
	std::string line;
	if (ifs.fail()) {
		std::cerr << "FAIL\n";
		return nullptr;
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	std::vector<std::array<unsigned int, 12>> faces;

	enum class ParserState {
		DEFAULT, POSITION, NORMAL, UV, FACE
	};

	while (getline(ifs, line)) {
		std::stringstream splitline(line);
		std::string word;
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		std::array<unsigned int, 12> face;
		size_t counter = 0;
		ParserState state = ParserState::DEFAULT;
		while (getline(splitline, word, ' ')) {
			// position
			if (counter == 0 && word.compare("v") == 0) {
				state = ParserState::POSITION;
			}
			else if (state == ParserState::POSITION && counter < 4) {
				position[counter - 1] = std::stof(word);

				if (counter == 3) positions.push_back(position);
			}
			// vertex uv coordinate
			else if (counter == 0 && word.compare("vt") == 0) {
				state = ParserState::UV;
			}
			else if (state == ParserState::UV && counter < 3) {
				uv[counter - 1] = std::stof(word);

				if (counter == 2) uvs.push_back(uv);
			}
			// vertex normal
			else if (counter == 0 && word.compare("vn") == 0) {
				state = ParserState::NORMAL;
			}
			else if (state == ParserState::NORMAL && counter < 4) {
				normal[counter - 1] = std::stof(word);

				if (counter == 3) normals.push_back(normal);
			}
			// face
			else if (counter == 0 && word.compare("f") == 0) {
				state = ParserState::FACE;
			}
			else if (state == ParserState::FACE && counter < 5) {
				std::stringstream splitslash(word);
				std::string strIndex;
				// position
				getline(splitslash, strIndex, '/');
				face[(counter - 1) * 3] = std::stoi(strIndex) - 1;
				// uv
				getline(splitslash, strIndex, '/');
				face[(counter - 1) * 3 + 1] = std::stoi(strIndex) - 1;
				// normal
				getline(splitslash, strIndex, '/');
				face[(counter - 1) * 3 + 2] = std::stoi(strIndex) - 1;

				if (counter == 4) faces.push_back(face);
			}
			counter++;
		}
	}
	ifs.close();

	glez::quad_mesh* mesh = new glez::quad_mesh();
	
	std::vector<std::shared_ptr<glez::vertex>> vertices(positions.size());
	for (size_t i = 0; i < positions.size(); i++) {
		vertices[i] = std::make_shared<glez::vertex>(positions[i]);
		mesh->add_vertex(vertices[i]);
	}

	std::unordered_map<std::shared_ptr<glez::half_edge>, glm::vec2> tex_coords;
	for (size_t i = 0; i < faces.size(); i++) {
		std::array<unsigned int, 12>& raw_face = faces[i];
		std::array<std::shared_ptr<glez::vertex>, 4> corners{
			vertices[raw_face[0]], vertices[raw_face[3]], vertices[raw_face[6]], vertices[raw_face[9]]
		};
		glm::vec3 normal = glm::normalize(
			normals[raw_face[2]] + normals[raw_face[5]] + normals[raw_face[8]] + normals[raw_face[11]]
		);
		std::shared_ptr<glez::quad_face> f = glez::quad_face::make_face(corners, normal);
		mesh->add_face(f);
		tex_coords[f->half_edges[0]] = uvs[raw_face[1]];
		tex_coords[f->half_edges[1]] = uvs[raw_face[4]];
		tex_coords[f->half_edges[2]] = uvs[raw_face[7]];
		tex_coords[f->half_edges[3]] = uvs[raw_face[10]];
	}

	obj->set_mesh(mesh);

	obj->set_texture(new glez::texture()); // TODO : load from file

	obj->unwrap(tex_coords);
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
