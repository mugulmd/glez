#include <memory>
#include <vector>
#include <array>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include <cmath>
#include <chrono>
#include <numeric>

#include "exp_scene.h"

#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool read_file(const char* filePath, std::string& fileText);
void load_OBJ(const char* objFilePath, glez::unwrapped_object* obj);
GLuint load_shader(const char* shaderPath, GLenum shaderType);
GLuint make_shader_program(std::vector<GLuint> shaderIDs);
glez::texture* load_texture(const char* imgFilePath);
float quad_area(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d);
void make_plane(glez::unwrapped_object* plane);

exp_scene::exp_scene(const char* obj_file_path, const char* img_file_path)
{
	glGenBuffers(4, m_vboIDs);
	glGenBuffers(4, m_eboIDs);
	glGenVertexArrays(4, m_vaoIDs);
	glGenTextures(4, m_texIDs);

	/* Shaders */
	GLuint vert_bland = load_shader("shaders/unwrap.vert", GL_VERTEX_SHADER);
	GLuint frag_bland = load_shader("shaders/unwrap.frag", GL_FRAGMENT_SHADER);
	m_shaderIDs[0] = make_shader_program({ vert_bland, frag_bland });

	GLuint vert_mc = load_shader("shaders/mesh_color.vert", GL_VERTEX_SHADER);
	GLuint frag_mc = load_shader("shaders/mesh_color.frag", GL_FRAGMENT_SHADER);
	m_shaderIDs[2] = make_shader_program({ vert_mc, frag_mc });

	m_shaderIDs[1] = m_shaderIDs[0];
	m_shaderIDs[3] = m_shaderIDs[0];

	/* Unwrapped object */
	m_obj_unwrapped = new glez::unwrapped_object(load_texture(img_file_path));
	load_OBJ(obj_file_path, m_obj_unwrapped);
	m_objs[0] = m_obj_unwrapped;

	/* Base (unwrapped) texture visualizer */
	m_plane_unwrapped = new glez::unwrapped_object(new glez::texture(
		m_obj_unwrapped->get_texture()->width(),
		m_obj_unwrapped->get_texture()->height(),
		m_obj_unwrapped->get_texture()->data()
	));
	make_plane(m_plane_unwrapped);
	m_objs[1] = m_plane_unwrapped;

	/* Mesh Color object */
	m_obj_mc = new glez::mc_object();
	transfer_res();
	m_obj_mc->pack_frames();
	transfer_colors();
	m_objs[2] = m_obj_mc;

	/* Mesh Color texture visualizer */
	m_plane_mc = new glez::unwrapped_object(new glez::texture(
		m_obj_mc->get_texture()->width(), 
		m_obj_mc->get_texture()->height(), 
		m_obj_mc->get_texture()->data()
	));
	make_plane(m_plane_mc);
	m_objs[3] = m_plane_mc;

	/* Graphics pipeline setup */
	for (size_t i = 0; i < 4; i++) {
		init_graphics(i, m_objs[i]->get_render_buffer()->uv_dim());
		m_objs[i]->create_uv_layout();
		send_to_gpu(i, m_objs[i]->get_render_buffer());
		send_to_gpu(i, m_objs[i]->get_texture());
	}
}

exp_scene::~exp_scene()
{
	delete m_obj_unwrapped;
	delete m_obj_mc;
}

unsigned int exp_scene::res_unwrapped(std::shared_ptr<glez::quad_face> face)
{
	// count number of pixels in quad
	// first approximation : use unwrapped quad area for counting pixels 
	float area = quad_area(
		m_obj_unwrapped->get_tex_coord(face->half_edges[0]),
		m_obj_unwrapped->get_tex_coord(face->half_edges[1]),
		m_obj_unwrapped->get_tex_coord(face->half_edges[2]),
		m_obj_unwrapped->get_tex_coord(face->half_edges[3])
	);
	float n_pixels = std::floor(area * m_obj_unwrapped->get_texture()->width() * m_obj_unwrapped->get_texture()->height());
	unsigned int r = (unsigned int)(std::ceil(std::log2(1.f + std::sqrt(n_pixels))));
	return std::pow(2, r);
}

void exp_scene::transfer_res()
{
	m_obj_mc->get_mesh()->get_vertices() = m_obj_unwrapped->get_mesh()->get_vertices();

	for (std::shared_ptr<glez::quad_face>& f : m_obj_unwrapped->get_mesh()->get_faces()) {
		unsigned int res = res_unwrapped(f);
		m_obj_mc->add_face(f, res, res);
	}
}

glm::u8vec4 exp_scene::color_unwrapped(std::shared_ptr<glez::quad_face> face, glm::vec2 coef)
{
	glm::vec2& a = m_obj_unwrapped->get_tex_coord(face->half_edges[0]);
	glm::vec2& b = m_obj_unwrapped->get_tex_coord(face->half_edges[1]);
	glm::vec2& c = m_obj_unwrapped->get_tex_coord(face->half_edges[2]);
	glm::vec2& d = m_obj_unwrapped->get_tex_coord(face->half_edges[3]);

	glm::vec2 tex_coord = a * (1 - coef.x) * (1 - coef.y)
		+ b * coef.x * (1 - coef.y)
		+ d * (1 - coef.x) * coef.y
		+ c * coef.x * coef.y;

	return m_obj_unwrapped->get_texture()->get_pixel(tex_coord.x, 1.f - tex_coord.y);
}

void exp_scene::transfer_colors()
{
	for (std::shared_ptr<glez::quad_face>& f : m_obj_mc->get_mesh()->get_faces()) {
		glez::frame& frame = m_obj_mc->get_frame(f);
		for (unsigned int x = 0; x <= frame.res.x; x++) {
			float alpha = (float)x / (float)frame.res.x;
			for (unsigned int y = 0; y <= frame.res.y; y++) {
				float beta = (float)y / (float)frame.res.y;
				frame.set_pixel(m_obj_mc->get_texture(),
					x, y, color_unwrapped(f, glm::vec2(alpha, beta)));
			}
		}
	}
}

void exp_scene::init_graphics(size_t idx, unsigned int uv_dim)
{
	unsigned int stride = 6 + uv_dim;

	glBindVertexArray(m_vaoIDs[idx]);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboIDs[idx]);
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coordinates
	glVertexAttribPointer(2, uv_dim, GL_FLOAT, GL_FALSE, stride * sizeof(float), (GLvoid*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eboIDs[idx]);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, m_texIDs[idx]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void exp_scene::send_to_gpu(size_t idx, glez::render_buffer* buffer)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vboIDs[idx]);
	glBufferData(GL_ARRAY_BUFFER,
		buffer->vertex_attributes().size() * sizeof(float),
		buffer->vertex_attributes().data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eboIDs[idx]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		buffer->face_indices().size() * sizeof(unsigned int),
		buffer->face_indices().data(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void exp_scene::send_to_gpu(size_t idx, glez::texture* texture)
{
	glBindTexture(GL_TEXTURE_2D, m_texIDs[idx]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		texture->width(), texture->height(),
		0,
		GL_RGBA, GL_UNSIGNED_BYTE,
		texture->data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void exp_scene::render(size_t idx, glez::abs_object* obj)
{
	glUseProgram(m_shaderIDs[idx]);
	GLint viewLoc = glGetUniformLocation(m_shaderIDs[idx], "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(get_camera()->get_view()));
	GLint projLoc = glGetUniformLocation(m_shaderIDs[idx], "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(get_camera()->get_proj()));

	glFinish();
	auto start = std::chrono::high_resolution_clock::now();

	glBindTexture(GL_TEXTURE_2D, m_texIDs[idx]);
	glBindVertexArray(m_vaoIDs[idx]);
	glDrawElements(GL_TRIANGLES,
		obj->get_render_buffer()->face_indices().size(),
		GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFinish();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> duration = end - start;

	if (m_n_frames < m_frame_times.size()) {
		m_frame_times[m_n_frames] = duration.count();
		m_n_frames++;
	}
	else {
		float avg = std::accumulate(m_frame_times.begin(), m_frame_times.end(), 0.f) / m_frame_times.size();
		m_n_frames = 0;
		if (m_log_rendering_time) {
			std::cout << "rendering time: " << avg * 1000.f << "ms" << std::endl;
		}
	}
}

void exp_scene::display()
{
	render(m_idx_display, m_objs[m_idx_display]);
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

void load_OBJ(const char* objFilePath, glez::unwrapped_object* obj)
{
	std::ifstream ifs(objFilePath, std::ios::in);
	std::string line;
	if (ifs.fail()) {
		std::cerr << "FAIL\n";
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
	
	std::vector<std::shared_ptr<glez::vertex>> vertices(positions.size());
	for (size_t i = 0; i < positions.size(); i++) {
		vertices[i] = std::make_shared<glez::vertex>(positions[i]);
		obj->get_mesh()->add_vertex(vertices[i]);
	}

	for (size_t i = 0; i < faces.size(); i++) {
		std::array<unsigned int, 12>& raw_face = faces[i];
		std::array<std::shared_ptr<glez::vertex>, 4> corners{
			vertices[raw_face[0]], vertices[raw_face[3]], vertices[raw_face[6]], vertices[raw_face[9]]
		};
		std::array<glm::vec2, 4> tex_coords{
			uvs[raw_face[1]], uvs[raw_face[4]], uvs[raw_face[7]], uvs[raw_face[10]]
		};
		glm::vec3 normal = glm::normalize(
			normals[raw_face[2]] + normals[raw_face[5]] + normals[raw_face[8]] + normals[raw_face[11]]
		);
		std::shared_ptr<glez::quad_face> f = glez::quad_face::make_face(corners, normal);
		obj->add_face(f, tex_coords);
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

glez::texture* load_texture(const char* imgFilePath)
{
	int w, h, n;
	unsigned char* img = stbi_load(imgFilePath, &w, &h, &n, 4);
	if (img == NULL) {
		std::cout << "ERROR::STB_IMAGE::LOADING_FAILED" << std::endl;
	}
	glez::texture* texture = new glez::texture(w, h, img);
	return texture;
}

float quad_area(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d)
{
	glm::vec2 diag1 = c - a;
	glm::vec2 diag2 = b - d;
	float dot = glm::dot(glm::normalize(diag1), glm::normalize(diag2));
	return 0.5f * glm::length(diag1) * glm::length(diag2) * std::sin(std::acos(dot));
}

void make_plane(glez::unwrapped_object* plane)
{
	plane->get_mesh()->add_vertex(std::make_shared<glez::vertex>(glm::vec3(-1, -1, 0)));
	plane->get_mesh()->add_vertex(std::make_shared<glez::vertex>(glm::vec3(-1, 1, 0)));
	plane->get_mesh()->add_vertex(std::make_shared<glez::vertex>(glm::vec3(1, 1, 0)));
	plane->get_mesh()->add_vertex(std::make_shared<glez::vertex>(glm::vec3(1, -1, 0)));
	plane->add_face(
		glez::quad_face::make_face(
			{ plane->get_mesh()->get_vertex(0),
			plane->get_mesh()->get_vertex(1),
			plane->get_mesh()->get_vertex(2),
			plane->get_mesh()->get_vertex(3) },
			glm::vec3(0, 0, 1)
		),
		{ glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0) }
	);
}
