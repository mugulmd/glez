#pragma once

#include <memory>
#include <unordered_set>

#include "glez.h"
#include "selection.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class op_displace
{
protected:
	std::unordered_set<std::shared_ptr<glez::vertex>> m_vertices;

public:
	op_displace(selection* sel);
	virtual ~op_displace() {}

	virtual void apply(glez::camera* cam, const glm::vec2& pick_start, const glm::vec2& pick_end) = 0;

};

class op_displace_free : public op_displace
{
public:
	op_displace_free(selection* sel);

	void apply(glez::camera* cam, const glm::vec2& pick_start, const glm::vec2& pick_end) override;

};

class op_displace_axis : public op_displace
{
private:
	glm::vec3 m_axis;

public:
	op_displace_axis(selection* sel, glm::vec3 axis);

	void apply(glez::camera* cam, const glm::vec2& pick_start, const glm::vec2& pick_end) override;

};

class op_extrude
{
private:
	glez::mc_object* m_obj;
	std::shared_ptr<glez::quad_face> m_face;

public:
	op_extrude(glez::mc_object* obj);

	void init(glez::camera* cam, const glm::vec2& pick_coords);
	void apply(glez::camera* cam, const glm::vec2& pick_start, const glm::vec2& pick_end);

	inline std::shared_ptr<glez::quad_face> get_face() { return m_face; }

};

class op_cut
{
private:
	glez::mc_object* m_obj;

public:
	op_cut(glez::mc_object* obj);

	void apply(glez::camera* cam, const glm::vec2& pick_coords);

};
