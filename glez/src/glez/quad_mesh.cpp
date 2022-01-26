#include "glez_pch.h"

#include "quad_mesh.h"

#include <glm/gtx/norm.hpp>

namespace glez {

	vertex::vertex(glm::vec3 _position) : 
		position(_position)
	{}

	half_edge::half_edge(std::shared_ptr<quad_face> _face, std::shared_ptr<vertex> _base) : 
		face(_face), base(_base)
	{}

	std::shared_ptr<quad_face> quad_face::make_face(std::array<std::shared_ptr<vertex>, 4> corners, glm::vec3 _normal)
	{
		std::shared_ptr<quad_face> f = std::make_shared<quad_face>();

		for (size_t i = 0; i < 4; i++) {
			f->half_edges[i] = std::make_shared<half_edge>(f, corners[i]);
		}

		for (size_t i = 0; i < 4; i++) {
			size_t i_next = (i + 1) % 4;
			f->half_edges[i]->next = f->half_edges[i_next];
		}
		
		f->normal = _normal;

		return f;
	}

	bool quad_face::pick(const ray& r, float& dist, glm::vec2& pick_coords)
	{
		glm::vec3& a = half_edges[0]->base->position;
		glm::vec3& b = half_edges[1]->base->position;
		glm::vec3& c = half_edges[3]->base->position;

		glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
		if (std::abs(glm::dot(n, r.direction)) < 0.001f) return false;

		float t = glm::dot(n, a - r.origin) / glm::dot(n, r.direction);
		if (t < 0.01f) return false;
		glm::vec3 p = r.at(t);

		float u = glm::dot(p - a, b - a);
		if (u < 0 || u > glm::length2(b - a)) return false;

		float v = glm::dot(p - a, c - a);
		if (v < 0 || v > glm::length2(c - a)) return false;

		dist = t;
		pick_coords.x = u / glm::length2(b - a);
		pick_coords.y = v / glm::length2(c - a);
		return true;
	}

	std::shared_ptr<half_edge> quad_face::nearest_half_edge(const glm::vec2& coords)
	{
		if (coords.x < 0.5f) {
			if (coords.y < coords.x)
				return half_edges[0];
			else if (coords.y < 0.5f)
				return half_edges[3];
			else if (coords.x < 1.f - coords.y)
				return half_edges[3];
			else
				return half_edges[2];
		}
		else {
			if (coords.y < 1.f - coords.x)
				return half_edges[0];
			else if (coords.y < 0.5f)
				return half_edges[1];
			else if (coords.y < coords.x)
				return half_edges[1];
			else
				return half_edges[2];
		}
	}

	std::shared_ptr<quad_face> quad_mesh::pick_face(const ray& r, float& dist, glm::vec2& pick_coords)
	{
		dist = -1.f;
		std::shared_ptr<quad_face> hit;
		for (std::shared_ptr<quad_face>& f : m_faces) {
			float tmp_dist;
			glm::vec2 tmp_coords;
			if (f->pick(r, tmp_dist, tmp_coords)) {
				if (dist < 0 || tmp_dist < dist) {
					dist = tmp_dist;
					pick_coords = tmp_coords;
					hit = f;
				}
			}
		}
		return hit;
	}

	std::shared_ptr<quad_face> quad_mesh::pick_face(const ray& r)
	{
		float dist;
		glm::vec2 pick_coords;
		return pick_face(r, dist, pick_coords);
	}

	std::shared_ptr<half_edge> quad_mesh::pick_edge(const ray& r)
	{
		float dist;
		glm::vec2 pick_coords;
		std::shared_ptr<quad_face> f = pick_face(r, dist, pick_coords);
		if (f) {
			return f->nearest_half_edge(pick_coords);
		}
		else {
			return nullptr;
		}
	}

	void quad_mesh::add_vertex(std::shared_ptr<vertex> vertex)
	{
		m_vertices.push_back(vertex);
	}

	void quad_mesh::add_face(std::shared_ptr<quad_face> face)
	{
		// TODO : connect half-edges using opposite pointers

		m_faces.push_back(face);
	}

	std::shared_ptr<quad_mesh> quad_mesh::make_cube()
	{
		std::shared_ptr<quad_mesh> mesh = std::make_shared<quad_mesh>();

		std::vector<glm::vec3> positions{
			glm::vec3(-1, 1, -1), 
			glm::vec3(-1, 1, 1),
			glm::vec3(-1, -1, 1),
			glm::vec3(-1, -1, -1),
			glm::vec3(1, 1, -1),
			glm::vec3(1, 1, 1),
			glm::vec3(1, -1, -1),
			glm::vec3(1, -1, 1)
		};

		std::vector<std::shared_ptr<vertex>> vertices(8);
		for (size_t i = 0; i < 8; i++) {
			vertices[i] = std::make_shared<vertex>(positions[i]);
			mesh->add_vertex(vertices[i]);
		}

		std::vector<std::array<size_t, 4>> corners_indices{
			{0, 1, 2, 3}, 
			{0, 4, 4, 1}, 
			{5, 4, 6, 7}, 
			{1, 5, 7, 2}, 
			{2, 7, 6, 3}, 
			{3, 6, 4, 0}
		};

		std::vector<glm::vec3> normals{
			glm::vec3(-1, 0, 0),
			glm::vec3(0, 1, 0),
			glm::vec3(1, 0, 0),
			glm::vec3(0, 0, 1),
			glm::vec3(0, -1, 0),
			glm::vec3(0, 0, -1)
		};

		for (size_t i = 0; i < 6; i++) {
			std::array<std::shared_ptr<vertex>, 4> corners;
			for (size_t j = 0; j < 4; j++) {
				corners[j] = vertices[corners_indices[i][j]];
			}
			std::shared_ptr<quad_face> f = quad_face::make_face(corners, normals[i]);
			mesh->add_face(f);
		}

		return mesh;
	}

}
