#pragma once

#include "Model/Geometry/GTriangle.h"

#include <vector>
#include <memory>

namespace ph
{

class GTriangleMesh final : public Geometry
{
public:
	virtual ~GTriangleMesh() override;

	virtual void discretize(std::vector<Triangle>* const out_triangles, const Model* const parentModel) const override;

	void addTriangle(const GTriangle gTriangle);

private:
	std::vector<GTriangle> m_gTriangles;
};

}// end namespace ph