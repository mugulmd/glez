#pragma once

#include <memory>
#include <array>
#include <list>
#include <vector>

#include "core.h"
#include "ray.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace glez {

	struct vertex;
	struct half_edge;
	struct quad_face;

	struct GLEZ_API vertex
	{
		glm::vec3 position;
		std::list<std::shared_ptr<half_edge>> half_edges;

		vertex(glm::vec3 _position);
	};

	struct GLEZ_API half_edge
	{
		std::shared_ptr<quad_face> face;
		size_t local_idx;
		std::shared_ptr<vertex> base;
		std::shared_ptr<half_edge> next;
		std::shared_ptr<half_edge> opposite;

		half_edge(std::shared_ptr<quad_face> _face, size_t idx, std::shared_ptr<vertex> _base);

		glm::vec3 mid_point();
	};

	struct GLEZ_API quad_face
	{
		std::array<std::shared_ptr<half_edge>, 4> half_edges;
		glm::vec3 normal;

		static std::shared_ptr<quad_face> make_face(std::array<std::shared_ptr<vertex>, 4> corners, glm::vec3 _normal);

		bool pick(const ray& r, float& dist, glm::vec2& pick_coords);
		std::shared_ptr<half_edge> nearest_half_edge(const glm::vec2& coords);
	};

	class GLEZ_API quad_mesh
	{
	private:
		std::vector<std::shared_ptr<vertex>> m_vertices;
		std::list<std::shared_ptr<quad_face>> m_faces;

	public:
		inline std::vector<std::shared_ptr<vertex>>& get_vertices() { return m_vertices; }
		inline size_t n_vertices() { return m_vertices.size(); }
		inline std::shared_ptr<vertex> get_vertex(size_t idx) { return m_vertices[idx]; }

		inline std::list<std::shared_ptr<quad_face>>& get_faces() { return m_faces; }

		std::shared_ptr<quad_face> pick_face(const ray& r, float& dist, glm::vec2& pick_coords);
		std::shared_ptr<quad_face> pick_face(const ray& r);
		std::shared_ptr<half_edge> pick_edge(const ray& r);

	public:
		void add_vertex(std::shared_ptr<vertex> vertex);
		void add_face(std::shared_ptr<quad_face> face);
		void remove_face(std::shared_ptr<quad_face> face);

	};

}
