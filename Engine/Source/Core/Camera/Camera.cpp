#include "Core/Camera/Camera.h"
#include "DataIO/SDL/InputPacket.h"
#include "Core/RayDifferential.h"
#include "Core/Ray.h"
#include "Common/Logger.h"
#include "Math/math.h"

#include <iostream>

namespace ph
{

namespace
{
	const Logger logger(LogSender("Camera"));
}

Camera::Camera() :
	Camera(math::Vector3R(0, 0, 0), math::QuaternionR::makeNoRotation())
{}

Camera::Camera(const math::Vector3R& position, const math::QuaternionR& rotation) :
	m_aspectRatio(16.0_r / 9.0_r)
{
	updateCameraPose(math::TVector3<hiReal>(position), math::TQuaternion<hiReal>(rotation));
}

void Camera::calcSensedRayDifferentials(
	const math::Vector2R& rasterPosPx, const Ray& sensedRay,
	RayDifferential* const out_result) const
{
	// 2nd-order accurate with respect to the size of <deltaPx>
	const real deltaPx        = 1.0_r / 32.0_r;
	const real reciIntervalPx = 1.0_r / deltaPx;

	Ray dnxRay, dpxRay, dnyRay, dpyRay;
	genSensedRay(math::Vector2R(rasterPosPx.x - deltaPx, rasterPosPx.y), &dnxRay);
	genSensedRay(math::Vector2R(rasterPosPx.x + deltaPx, rasterPosPx.y), &dpxRay);
	genSensedRay(math::Vector2R(rasterPosPx.x, rasterPosPx.y - deltaPx), &dnyRay);
	genSensedRay(math::Vector2R(rasterPosPx.x, rasterPosPx.y + deltaPx), &dpyRay);

	out_result->setPartialPs((dpxRay.getOrigin() - dnxRay.getOrigin()).divLocal(reciIntervalPx),
	                         (dpyRay.getOrigin() - dnyRay.getOrigin()).divLocal(reciIntervalPx));

	out_result->setPartialDs((dpxRay.getDirection() - dnxRay.getDirection()).divLocal(reciIntervalPx),
	                         (dpyRay.getDirection() - dnyRay.getDirection()).divLocal(reciIntervalPx));
}

void Camera::updateCameraPose(const math::TVector3<hiReal>& position, const math::TQuaternion<hiReal>& rotation)
{
	m_cameraToWorldTransform.setPosition(position);

	// changes unit from mm to m
	m_cameraToWorldTransform.setScale(math::TVector3<hiReal>(0.001));

	m_cameraToWorldTransform.setRotation(rotation);

	m_position  = math::Vector3R(position);
	m_direction = math::Vector3R(math::TVector3<hiReal>(0, 0, -1).rotate(rotation).normalize());
}

math::TQuaternion<hiReal> Camera::getWorldToCameraRotation(const math::Vector3R& direction, const math::Vector3R& upAxis)
{
	constexpr hiReal MIN_LENGTH = 0.001;

	auto zAxis = math::TVector3<hiReal>(direction).mul(-1.0f);
	if(zAxis.lengthSquared() > MIN_LENGTH * MIN_LENGTH)
	{
		zAxis.normalizeLocal();
	}
	else
	{
		logger.log(ELogLevel::WARNING_MED,
			"Direction vector " + direction.toString() + " is too short. "
			"Defaults to -z axis.");

		zAxis.set(0, 0, -1);
	}

	auto xAxis = math::TVector3<hiReal>(upAxis).cross(zAxis);
	auto yAxis = zAxis.cross(xAxis);
	if(xAxis.lengthSquared() > MIN_LENGTH * MIN_LENGTH)
	{
		xAxis.normalizeLocal();
		yAxis.normalizeLocal();
	}
	else
	{
		logger.log(ELogLevel::WARNING_MED,
			"Up axis " + upAxis.toString() + "is not properly configured. "
			"It is too close to the direction vector, or its length is too short. "
			"Trying to recover using +x as right vector or +y as up vector.");

		if(zAxis.absDot({1, 0, 0}) < zAxis.absDot({0, 1, 0}))
		{
			yAxis = zAxis.cross({1, 0, 0}).normalizeLocal();
			xAxis = yAxis.cross(zAxis).normalizeLocal();
		}
		else
		{
			xAxis = math::TVector3<hiReal>(0, 1, 0).cross(zAxis).normalizeLocal();
			yAxis = zAxis.cross(xAxis).normalizeLocal();
		}
	}

	const auto worldToViewRotMat = math::TMatrix4<hiReal>().initRotation(xAxis, yAxis, zAxis);
	return math::TQuaternion<hiReal>(worldToViewRotMat);
}

math::TQuaternion<hiReal> Camera::getWorldToCameraRotation(real yawDegrees, real pitchDegrees)
{
	if(yawDegrees < 0.0_r || yawDegrees > 360.0_r)
	{
		logger.log(ELogLevel::WARNING_MED,
			"Yaw degree " + std::to_string(yawDegrees) + " out of range. Clamping to [0, 360]");

		yawDegrees = math::clamp(yawDegrees, 0.0_r, 360.0_r);
	}

	if(pitchDegrees < -90.0_r || pitchDegrees > 90.0_r)
	{
		logger.log(ELogLevel::WARNING_MED,
			"Pitch degree " + std::to_string(pitchDegrees) + " out of range. Clamping to [-90, 90]");

		pitchDegrees = math::clamp(pitchDegrees, -90.0_r, 90.0_r);
	}

	const math::TQuaternion<hiReal> yawRot({0, 1, 0}, math::to_radians(yawDegrees));
	const math::TQuaternion<hiReal> pitchRot({1, 0, 0}, math::to_radians(pitchDegrees));
	return yawRot.mul(pitchRot).normalize();
}

// command interface

Camera::Camera(const InputPacket& packet) :
	Camera()
{
	const auto position = packet.getVector3("position", 
		math::Vector3R(0), DataTreatment::REQUIRED());

	auto rotation = math::TQuaternion<hiReal>::makeNoRotation();
	if(packet.hasQuaternion("rotation"))
	{
		rotation = math::TQuaternion<hiReal>(packet.getQuaternion("rotation"));
	}
	else if(packet.hasVector3("direction") && packet.hasVector3("up-axis"))
	{
		const auto direction = packet.getVector3("direction");
		const auto upAxis    = packet.getVector3("up-axis");
		rotation = getWorldToCameraRotation(direction, upAxis);
	}
	else if(packet.hasReal("yaw-degrees") && packet.hasReal("pitch-degrees"))
	{
		rotation = getWorldToCameraRotation(packet.getReal("yaw-degrees"), packet.getReal("pitch-degrees"));
	}
	else
	{
		logger.log(ELogLevel::WARNING_MED, "no camera rotation info provided");
	}

	updateCameraPose(math::TVector3<hiReal>(position), rotation);
}

SdlTypeInfo Camera::ciTypeInfo()
{
	return SdlTypeInfo(ETypeCategory::REF_CAMERA, "camera");
}

void Camera::ciRegister(CommandRegister& cmdRegister) {}

}// end namespace ph
