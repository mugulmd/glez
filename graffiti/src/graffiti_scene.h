#pragma once

#include "GL/glew.h"

#include "glez.h"
#include "render_engine.h"
#include "selection.h"
#include "painting_ops.h"
#include "modeling_ops.h"

#include <glm/vec2.hpp>

class graffiti_scene : 
	public glez::scene
{
	/* Mesh Color Object */
private:
	glez::mc_object* m_obj;

	void init_cube();

public:
	graffiti_scene();
	~graffiti_scene();

	inline void log_object_info() { m_obj->log_info(); }

	/* Rendering */
private:
	render_engine* m_renderer;
	bool m_show_selection;

public:
	void display();
	inline void show_selection() { m_show_selection = true; }

	/* Selection */
private:
	selection* m_selection;

public:
	inline selection* get_selection() { return m_selection; }
	void set_selection(const glm::vec2& pick_coords);
	void add_to_selection(const glm::vec2& pick_coords);
	void select_all();
	void invert_selection();

	/* Painting Operations */
private:
	op_paint* m_painter;

public:
	void set_painter_fill();
	void set_painter_spray();
	void paint(const glm::vec2& pick_coords);

	/* Modeling Operations */
private:
	op_extrude* m_extruder;
	op_cut* m_cutter;

public:
	void set_extrude();
	void init_extrude(const glm::vec2& pick_coords);
	void extrude(const glm::vec2& pick_start, const glm::vec2& pick_end);
	void set_cut();
	void cut(const glm::vec2& pick_coords);

};
