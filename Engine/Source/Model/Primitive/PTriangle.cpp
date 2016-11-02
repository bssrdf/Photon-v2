#include "Model/Primitive/PTriangle.h"
#include "Core/Ray.h"
#include "Core/Intersection.h"

#include <limits>
#include <cmath>

#define TRIANGLE_EPSILON 0.0001f

namespace ph
{

PTriangle::PTriangle(const Vector3f& vA, const Vector3f& vB, const Vector3f& vC, const Model* const parentModel) :
	Primitive(parentModel), 
	m_vA(vA), m_vB(vB), m_vC(vC)
{
	m_eAB = vB.sub(vA);
	m_eAC = vC.sub(vA);

	m_faceNormal = m_eAB.cross(m_eAC).normalizeLocal();
}

PTriangle::~PTriangle() = default;

bool PTriangle::isIntersecting(const Ray& ray, Intersection* const out_intersection) const
{
	const float32 dist = ray.getOrigin().sub(m_vA).dot(m_faceNormal) / (-ray.getDirection().dot(m_faceNormal));

	// reject by distance
	if(dist < TRIANGLE_EPSILON || dist > std::numeric_limits<float32>::max() || dist != dist)
		return false;

	// projected hit point
	float32 hitPu, hitPv;

	// projected side vector AB and AC
	float32 abPu, abPv, acPu, acPv;

	// find dominant axis
	if(abs(m_faceNormal.x) > abs(m_faceNormal.y))
	{
		// X dominant, projection plane is YZ
		if(abs(m_faceNormal.x) > abs(m_faceNormal.z))
		{
			hitPu = dist * ray.getDirection().y + ray.getOrigin().y - m_vA.y;
			hitPv = dist * ray.getDirection().z + ray.getOrigin().z - m_vA.z;
			abPu = m_eAB.y;
			abPv = m_eAB.z;
			acPu = m_eAC.y;
			acPv = m_eAC.z;
		}
		// Z dominant, projection plane is XY
		else
		{
			hitPu = dist * ray.getDirection().x + ray.getOrigin().x - m_vA.x;
			hitPv = dist * ray.getDirection().y + ray.getOrigin().y - m_vA.y;
			abPu = m_eAB.x;
			abPv = m_eAB.y;
			acPu = m_eAC.x;
			acPv = m_eAC.y;
		}
	}
	// Y dominant, projection plane is ZX
	else if(abs(m_faceNormal.y) > abs(m_faceNormal.z))
	{
		hitPu = dist * ray.getDirection().z + ray.getOrigin().z - m_vA.z;
		hitPv = dist * ray.getDirection().x + ray.getOrigin().x - m_vA.x;
		abPu = m_eAB.z;
		abPv = m_eAB.x;
		acPu = m_eAC.z;
		acPv = m_eAC.x;
	}
	// Z dominant, projection plane is XY
	else
	{
		hitPu = dist * ray.getDirection().x + ray.getOrigin().x - m_vA.x;
		hitPv = dist * ray.getDirection().y + ray.getOrigin().y - m_vA.y;
		abPu = m_eAB.x;
		abPv = m_eAB.y;
		acPu = m_eAC.x;
		acPv = m_eAC.y;
	}

	// TODO: check if these operations are possible of producing NaNs

	// barycentric coordinate of vertex B in the projected plane
	const float32 baryB = (hitPu*acPv - hitPv*acPu) / (abPu*acPv - abPv*acPu);
	if(baryB < 0.0f) return false;

	// barycentric coordinate of vertex C in the projected plane
	const float32 baryC = (hitPu*abPv - hitPv*abPu) / (acPu*abPv - abPu*acPv);
	if(baryC < 0.0f) return false;

	if(baryB + baryC > 1.0f) return false;

	// so the ray intersects the triangle (TODO: reuse calculated results!)

	out_intersection->setHitPosition(ray.getDirection().mul(dist).addLocal(ray.getOrigin()));
	out_intersection->setHitNormal(m_faceNormal);
	out_intersection->setHitPrimitive(this);

	return true;
}

}// end namespace ph