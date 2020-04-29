#pragma once

#include "Common/primitive_type.h"
#include "Math/TVector2.h"
#include "Math/TVector3.h"
#include "Math/TQuaternion.h"
#include "DataIO/SDL/TCommandInterface.h"
#include "Math/Transform/TDecomposedTransform.h"
#include "Core/Filmic/filmic_fwd.h"
#include "Common/assertion.h"

namespace ph
{

class Ray;
class Sample;
class SampleGenerator;
class InputPacket;
class RayDifferential;

namespace math
{
	class Transform;
}

class Camera : public TCommandInterface<Camera>
{
public:
	Camera();

	Camera(
		const math::Vector3R&    position, 
		const math::QuaternionR& rotation,
		const math::Vector2S&    resolution);

	virtual ~Camera() = default;

	// TODO: consider changing <filmNdcPos> to 64-bit float

	// Given a NDC position on the film, generate a corresponding ray
	// that would have hit that position from the light entry of the camera (i.e., 
	// the furthest plane parallel to film the camera's lens system can reach).
	virtual void genSensedRay(const math::Vector2R& filmNdcPos, Ray* out_ray) const = 0;

	// Given a ray generated by genSensedRay() along with the parameters for it, 
	// calculates differential information on the origin of the ray.
	// The default implementation uses numerical differentiation for 
	// the differentials.
	/*virtual void calcSensedRayDifferentials(const math::Vector2R& filmNdcPos, const Ray& sensedRay,
	                                        RayDifferential* out_result) const;*/

	virtual void evalEmittedImportanceAndPdfW(const math::Vector3R& targetPos, math::Vector2R* const out_filmCoord, math::Vector3R* const out_importance, real* out_filmArea, real* const out_pdfW) const = 0;

	const math::Vector3R& getPosition() const;
	const math::Vector3R& getDirection() const;
	const math::Vector2S& getResolution() const;
	real getAspectRatio() const;

protected:
	math::Vector3R                   m_position;
	math::Vector3R                   m_direction;
	math::Vector2S                   m_resolution;
	math::TDecomposedTransform<real> m_decomposedCameraPose;

private:
	static math::Vector3R makeDirectionFromRotation(const math::QuaternionR& rotation);
	static math::QuaternionR makeRotationFromYawPitch(real yawDegrees, real pitchDegrees);

	static math::QuaternionR makeRotationFromVectors(
		const math::Vector3R& direction, 
		const math::Vector3R& upAxis);

	static math::TDecomposedTransform<real> makeDecomposedCameraPose(
		const math::Vector3R&    position, 
		const math::QuaternionR& rotation);

// command interface
public:
	explicit Camera(const InputPacket& packet);
	static SdlTypeInfo ciTypeInfo();
	static void ciRegister(CommandRegister& cmdRegister);
};

// In-header Implementations:

inline const math::Vector3R& Camera::getPosition() const
{
	return m_position;
}

inline const math::Vector3R& Camera::getDirection() const
{
	return m_direction;
}

inline const math::Vector2S& Camera::getResolution() const
{
	return m_resolution;
}

inline real Camera::getAspectRatio() const
{
	PH_ASSERT_GT(m_resolution.y, 0);

	return static_cast<real>(m_resolution.x) / static_cast<real>(m_resolution.y);
}

}// end namespace ph

/*
	<SDL_interface>

	<category>  camera </category>
	<type_name> camera </type_name>

	<name> Camera </name>
	<description>
		A camera for observing the scene.
	</description>

	<command type="creator" intent="blueprint">

		<input name="position" type="vector3">
			<description>Position of the camera.</description>
		</input>

		<input name="rotation" type="quaternion">
			<description>The orientation of the camera.</description>
		</input>

		<input name="direction" type="vector3">
			<description>Direction that this camera is looking at.</description>
		</input>
		<input name="up-axis" type="vector3">
			<description>The direction that this camera consider as upward.</description>
		</input>

		<input name="yaw-degrees" type="real">
			<description>Rotation of the camera around +y axis in [0, 360].</description>
		</input>
		<input name="pitch-degrees" type="real">
			<description>The camera's declination from the horizon in [-90, 90].</description>
		</input>

	</command>

	</SDL_interface>
*/
