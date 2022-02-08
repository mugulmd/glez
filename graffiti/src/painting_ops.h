#pragma once

#include "glez.h"

#include <glm/vec2.hpp>

class op_paint
{
protected:
	glez::mc_object* m_obj;

public:
	static float color[4];

	op_paint(glez::mc_object* obj);
	virtual ~op_paint() {}

	virtual void apply(glez::camera* cam, const glm::vec2& pick_coords) = 0;

};

class op_fill : public op_paint
{
public:
	op_fill(glez::mc_object* obj);

	void apply(glez::camera* cam, const glm::vec2& pick_coords) override;

};

class op_spray : public op_paint
{
public:
	static float radius;
	static int n_ray;

	op_spray(glez::mc_object* obj);

	void apply(glez::camera* cam, const glm::vec2& pick_coords) override;

};
