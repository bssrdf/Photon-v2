#include "Core/Intersectable/PSphere.h"
#include "Common/assertion.h"
#include "Math/constant.h"
#include "Core/Bound/AABB3D.h"
#include "Math/Math.h"
#include "Math/TVector3.h"
#include "Core/Ray.h"
#include "Core/HitProbe.h"
#include "Core/HitDetail.h"
#include "Core/Intersectable/PrimitiveMetadata.h"
#include "Core/Intersectable/UvwMapper/UvwMapper.h"
#include "Math/TMatrix2.h"
#include "Math/sampling.h"
#include "Math/Random.h"
#include "Core/Sample/PositionSample.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ph
{

PSphere::PSphere(const PrimitiveMetadata* const metadata, const real radius) : 
	Primitive(metadata),
	m_radius(radius), m_reciRadius(radius > 0.0_r ? 1.0_r / radius : 0.0_r)
{
	PH_ASSERT(radius > 0.0_r);
}

PSphere::~PSphere() = default;

bool PSphere::isIntersecting(const Ray& ray, HitProbe& probe) const
{
	// ray origin:         o
	// ray direction:      d
	// sphere center:      c
	// sphere radius:      r
	// intersection point: p
	// vector dot:         *
	// ray equation:       o + td (t is a scalar variable)
	//
	// To find the intersection point, the length of vector (td - oc) must equals r.
	// This is equivalent to (td - oc)*(td - oc) = r^2. After reformatting, we have
	//
	//              t^2(d*d) - 2t(d*op) + (oc*oc) - r^2 = 0     --- (1)
	//
	// Solving equation (1) for t will yield the intersection point (o + td).

	// vector from ray origin to sphere center
	//
	const Vector3R& oc = Vector3R(0, 0, 0).sub(ray.getOrigin());

	const real a = ray.getDirection().dot(ray.getDirection());// a in equation (1)
	const real b = ray.getDirection().dot(oc);                // b in equation (1) (-2 is cancelled while solving t)
	const real c = oc.dot(oc) - m_radius * m_radius;          // c in equation (1)

	real D = b * b - a * c;
	if(D < 0.0_r)
	{
		return false;
	}
	else
	{
		D = std::sqrt(D);

		const real reciA = 1.0_r / a;

		// pick the closest point in front of ray tail
		// t = (b +- D) / a
		//
		real t = 0.0_r;
		if((t = (b - D) * reciA) > ray.getMinT())
		{
			probe.pushBaseHit(this, t);
			return true;
		}
		else if((t = (b + D) * reciA) > ray.getMinT())
		{
			probe.pushBaseHit(this, t);
			return true;
		}
		else
		{
			return false;
		}
	}
}

void PSphere::calcIntersectionDetail(
	const Ray&       ray, 
	HitProbe&        probe,
	HitDetail* const out_detail) const
{
	PH_ASSERT(m_metadata != nullptr);
	const UvwMapper* mapper = m_metadata->getChannel(probe.getChannel()).getMapper();

	const Vector3R& hitPosition = ray.getOrigin().add(ray.getDirection().mul(probe.getHitRayT()));
	const Vector3R& hitNormal   = hitPosition.mul(m_reciRadius);

	PH_ASSERT(mapper != nullptr);
	Vector3R hitUvw;
	mapper->map(hitPosition, &hitUvw);

	out_detail->getHitInfo(ECoordSys::LOCAL).setAttributes(
		hitPosition, 
		hitNormal,
		hitNormal,
		probe.getHitRayT());

	// compute partial derivatives using 2nd-order approximation

	// calculating displacement vectors on hit normals tangent plane
	//
	const real delta = m_radius / 128.0_r;
	Vector3R dx, dz;
	Math::formOrthonormalBasis(hitNormal, &dx, &dz);
	dx.mulLocal(delta);
	dz.mulLocal(delta);

	// find delta positions on the sphere from displacement vectors
	//
	const Vector3R& negX = hitPosition.sub(dx).normalizeLocal().mulLocal(m_radius);
	const Vector3R& posX = hitPosition.add(dx).normalizeLocal().mulLocal(m_radius);
	const Vector3R& negZ = hitPosition.sub(dz).normalizeLocal().mulLocal(m_radius);
	const Vector3R& posZ = hitPosition.add(dz).normalizeLocal().mulLocal(m_radius);

	// find delta uvw vectors
	//
	Vector3R negXuvw, posXuvw, negZuvw, posZuvw;
	mapper->map(negX, &negXuvw);
	mapper->map(posX, &posXuvw);
	mapper->map(negZ, &negZuvw);
	mapper->map(posZ, &posZuvw);

	// calculating positional partial derivatives
	//
	Vector3R dPdU, dPdV;
	const Matrix2R uvwDiff(posXuvw.x - negXuvw.x, posXuvw.y - negXuvw.y,
	                       posZuvw.x - negZuvw.x, posZuvw.y - negZuvw.y);
	if(!uvwDiff.solve(posX.sub(negX), posZ.sub(negZ), &dPdU, &dPdV))
	{
		Math::formOrthonormalBasis(hitNormal, &dPdU, &dPdV);
	}

	// normal derivatives are actually scaled version of dPdU and dPdV
	//
	const Vector3R& dNdU = dPdU.mul(m_reciRadius);
	const Vector3R& dNdV = dPdV.mul(m_reciRadius);

	out_detail->getHitInfo(ECoordSys::LOCAL).setDerivatives(
		dPdU, dPdV, dNdU, dNdV);

	out_detail->getHitInfo(ECoordSys::WORLD) = out_detail->getHitInfo(ECoordSys::LOCAL);
	out_detail->setMisc(this, hitUvw);
}

// Intersection test for solid box and hollow sphere.
// Reference: Jim Arvo's algorithm in Graphics Gems 2
//
bool PSphere::isIntersectingVolumeConservative(const AABB3D& volume) const
{
	const real radius2 = Math::squared(m_radius);

	// These variables are gonna store minimum and maximum squared distances 
	// from the sphere's center to the AABB volume.
	//
	real minDist2 = 0.0_r;
	real maxDist2 = 0.0_r;

	real a, b;

	a = Math::squared(volume.getMinVertex().x);
	b = Math::squared(volume.getMaxVertex().x);
	maxDist2 += std::max(a, b);
	if     (0.0_r < volume.getMinVertex().x) minDist2 += a;
	else if(0.0_r > volume.getMaxVertex().x) minDist2 += b;

	a = Math::squared(volume.getMinVertex().y);
	b = Math::squared(volume.getMaxVertex().y);
	maxDist2 += std::max(a, b);
	if     (0.0_r < volume.getMinVertex().y) minDist2 += a;
	else if(0.0_r > volume.getMaxVertex().y) minDist2 += b;

	a = Math::squared(volume.getMinVertex().z);
	b = Math::squared(volume.getMaxVertex().z);
	maxDist2 += std::max(a, b);
	if     (0.0_r < volume.getMinVertex().z) minDist2 += a;
	else if(0.0_r > volume.getMaxVertex().z) minDist2 += b;

	return minDist2 <= radius2 && radius2 <= maxDist2;
}

void PSphere::calcAABB(AABB3D* const out_aabb) const
{
	out_aabb->setMinVertex(Vector3R(-m_radius, -m_radius, -m_radius));
	out_aabb->setMaxVertex(Vector3R( m_radius,  m_radius,  m_radius));
	out_aabb->expand(Vector3R(0.0001_r * m_radius));
}

real PSphere::calcPositionSamplePdfA(const Vector3R& position) const
{
	return 1.0_r / this->PSphere::calcExtendedArea();
}

void PSphere::genPositionSample(PositionSample* const out_sample) const
{
	PH_ASSERT(m_metadata != nullptr);

	sampling::unit_sphere::uniform::gen(
		Random::genUniformReal_i0_e1(), Random::genUniformReal_i0_e1(), &(out_sample->position));
	out_sample->normal = out_sample->position;
	out_sample->position.mulLocal(m_radius);

	// FIXME: able to specify mapper channel
	const UvwMapper* mapper = m_metadata->getDefaultChannel().getMapper();
	PH_ASSERT(mapper != nullptr);
	mapper->map(out_sample->position, &(out_sample->uvw));

	out_sample->pdf = this->calcPositionSamplePdfA(out_sample->position);
}

real PSphere::calcExtendedArea() const
{
	return 4.0_r * PH_PI_REAL * m_radius * m_radius;
}

}// end namespace ph