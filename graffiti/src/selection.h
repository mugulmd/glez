#pragma once

#include <unordered_set>
#include <vector>

#include "glez.h"

class selection 
{
private:
	glez::mc_object* m_obj;

public:
	selection(glez::mc_object* obj);
	~selection();

	inline glez::mc_object* get_obj() { return m_obj; }

private:
	std::unordered_set<std::shared_ptr<glez::quad_face>> m_faces;

public:
	inline std::unordered_set<std::shared_ptr<glez::quad_face>>& get_faces() { return m_faces; }
	void set(std::shared_ptr<glez::quad_face> face);
	void add(std::shared_ptr<glez::quad_face> face);
	void all();
	void inverse();
	void clear();

private:
	std::vector<unsigned int> m_indices;

public:
	inline std::vector<unsigned int>& get_indices() { return m_indices; }
	void update_indices();

};
