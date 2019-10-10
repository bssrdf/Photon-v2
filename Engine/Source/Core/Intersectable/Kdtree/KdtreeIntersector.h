#pragma once

#include "Core/Intersectable/Intersector.h"
#include "Core/Intersectable/Kdtree/KdtreeNode.h"
#include "Core/Intersectable/Intersectable.h"

#include <vector>

namespace ph
{

class KdtreeIntersector : public Intersector
{
public:
	KdtreeIntersector();
	virtual ~KdtreeIntersector() override;

	void update(const CookedDataStorage& cookedActors) override;
	bool isIntersecting(const Ray& ray, HitProbe& probe) const override;
	void calcAABB(math::AABB3D* out_aabb) const override;

private:
	std::vector<const Intersectable*> m_nodeIntersectableBuffer;
	KdtreeNode m_rootKdtreeNode;
};

}// end namespace ph
