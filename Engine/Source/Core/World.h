#pragma once

#include "Model/Primitive/Primitive.h"
#include "Model/Model.h"

#include <vector>
#include <memory>

namespace ph
{

class Intersection;
class Ray;

class World final
{
public:
	void addModel(const Model& model);
	bool isIntersecting(const Ray& ray, Intersection* out_intersection) const;

	void cook();

private:
	std::vector<std::unique_ptr<Model>> m_models;
	std::vector<std::unique_ptr<Primitive>> m_primitives;
};

}// end namespace ph