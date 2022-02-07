#include "selection.h"

selection::selection(glez::abs_object* obj) : 
	m_obj(obj)
{}

selection::~selection()
{}

void selection::set(std::shared_ptr<glez::quad_face> face)
{
	if (m_faces.size() == 0) {
		m_faces.insert(face);
		return;
	}
	else if (m_faces.size() == 1 && *(m_faces.begin()) == face) {
		return;
	}
	else {
		m_faces.clear();
		m_faces.insert(face);
		return;
	}
}

void selection::add(std::shared_ptr<glez::quad_face> face)
{
	m_faces.insert(face);
}

void selection::all()
{
	for (std::shared_ptr<glez::quad_face>& f : m_obj->get_mesh()->get_faces()) {
		add(f);
	}
}

void selection::inverse()
{
	std::unordered_set<std::shared_ptr<glez::quad_face>> new_faces;

	for (std::shared_ptr<glez::quad_face>& f : m_obj->get_mesh()->get_faces()) {
		if (m_faces.find(f) == m_faces.end()) {
			new_faces.insert(f);
		}
	}

	m_faces = new_faces;
}

void selection::update_indices()
{
	m_indices = m_obj->get_render_buffer()->sub_face_indices(m_faces.begin(), m_faces.end());
}
