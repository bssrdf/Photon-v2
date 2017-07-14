#pragma once

#include "Math/math_fwd.h"

#include <memory>

namespace ph
{

class Ray;
class Time;
class Intersection;
class AABB;

class Transform
{
public:
	virtual ~Transform() = 0;

	virtual std::unique_ptr<Transform> genInversed() const;

	// Treating a Vector3R as either
	//
	// 1) vectors (V), 
	// 2) orientations, such as normals and tangents (O), 
	// 3) points (P) 
	//
	// and calculate the transformed result.

	void transformV(const Vector3R& vector,      Vector3R* out_vector)      const;
	void transformO(const Vector3R& orientation, Vector3R* out_orientation) const;
	void transformP(const Vector3R& point,       Vector3R* out_point)       const;
	void transformV(const Vector3R& vector,      const Time& time, Vector3R* out_vector)      const;
	void transformO(const Vector3R& orientation, const Time& time, Vector3R* out_orientation) const;
	void transformP(const Vector3R& point,       const Time& time, Vector3R* out_point)       const;

	// Notice that transforming a ray neither will change its parametric 
	// length (t) nor renormalizing its direction vector even if the transform 
	// contains scale factor; because if users respect the ray segment's 
	// definition:
	// 
	//     Ray Segment = [rayOrigin + rayMinT * rayDirection, 
	//                    rayOrigin + rayMaxT * rayDirection]
	// 
	// this operation will always yield a correctly transformed result while 
	// saving an expensive sqrt() call.
	void transform(const Ray& ray, Ray* out_ray) const;

	void transform(const Intersection& intersection, Intersection* out_intersection) const;
	void transform(const AABB& aabb, AABB* out_aabb) const;
	void transform(const Intersection& intersection, const Time& time, 
	               Intersection* out_intersection) const;
	void transform(const AABB& aabb, const Time& time, 
	               AABB* out_aabb) const;

private:

	// Treating a Vector3R as either a vector, orientation, or point and 
	// calculate the transformed result.

	virtual void transformVector(const Vector3R& vector, const Time& time, 
	                             Vector3R* out_vector) const = 0;

	virtual void transformOrientation(const Vector3R& orientation, const Time& time,
	                                  Vector3R* out_orientation) const = 0;

	virtual void transformPoint(const Vector3R& point, const Time& time, 
	                            Vector3R* out_point) const = 0;

	// Transform the specified line segment. A line segment's definition is 
	// as follows:
	// 
	//     Line Segment = [lineStartPos + lineMinT * lineDir,
	//                     lineStartPos + lineMaxT * lineDir]
	//
	// Also note that lineDir is not necessary to be normalized.
	virtual void transformLineSegment(const Vector3R& lineStartPos, const Vector3R& lineDir, 
	                                  real lineMinT, real lineMaxT, const Time& time, 
	                                  Vector3R* out_lineStartPos, Vector3R* out_lineDir,
	                                  real* out_lineMinT, real* out_lineMaxT) const = 0;
};

}// end namespace ph