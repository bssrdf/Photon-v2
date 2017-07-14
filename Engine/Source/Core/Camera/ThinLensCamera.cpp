#include "Core/Camera/ThinLensCamera.h"
#include "Core/Camera/Film.h"
#include "Core/Sample.h"
#include "Core/Ray.h"
#include "Math/Transform/Transform.h"
#include "FileIO/InputPacket.h"
#include "Math/Random.h"

#include <iostream>

namespace ph
{

ThinLensCamera::~ThinLensCamera() = default;

void ThinLensCamera::genSensingRay(const Sample& sample, Ray* const out_ray) const
{
	const Vector3R rasterPosPx(sample.m_cameraX * getFilm()->getWidthPx(),
	                           sample.m_cameraY * getFilm()->getHeightPx(),
	                           0);
	Vector3R camFilmPos;
	m_rasterToCamera->transformP(rasterPosPx, &camFilmPos);

	const Vector3R camCenterRayDir = camFilmPos.mul(-1);
	const real     hitParamDist    = m_focalDistanceMM / (-camCenterRayDir.z);
	const Vector3R camFocusPos     = camCenterRayDir.mul(hitParamDist);

	//std::cerr << camFilmPos.toStringFormal() << std::endl;

	Vector3R camLensPos;
	genRandomSampleOnDisk(m_lensRadiusMM, &camLensPos.x, &camLensPos.y);

	

	Vector3R worldLensPos, worldFocusPos;
	m_cameraToWorld->transformP(camLensPos,  &worldLensPos);
	m_cameraToWorld->transformP(camFocusPos, &worldFocusPos);


	out_ray->setDirection(worldLensPos.sub(worldFocusPos).normalizeLocal());
	out_ray->setOrigin(worldLensPos);
	out_ray->setMinT(0.0001_r);// HACK: hard-coded number
	out_ray->setMaxT(Ray::MAX_T);
}

void ThinLensCamera::evalEmittedImportanceAndPdfW(
	const Vector3R& targetPos,
	Vector2f* const out_filmCoord,
	Vector3R* const out_importance,
	real* out_filmArea,
	real* const out_pdfW) const
{
	std::cerr << "ThinLensCamera::evalEmittedImportanceAndPdfW() not implemented" << std::endl;
}

void ThinLensCamera::genRandomSampleOnDisk(const real radius, real* const out_x, real* const out_y)
{
	const real r   = radius * std::sqrt(Random::genUniformReal_i0_e1());
	const real phi = 2.0_r * PI_REAL * Random::genUniformReal_i0_e1();
	*out_x = r * std::cos(phi);
	*out_y = r * std::sin(phi);
}

// command interface

ThinLensCamera::ThinLensCamera(const InputPacket& packet) : 
	PerspectiveCamera(packet), 
	m_lensRadiusMM(0.0_r), m_focalDistanceMM()
{
	m_lensRadiusMM    = packet.getReal("lens-radius-mm",    m_lensRadiusMM,    DataTreatment::REQUIRED());
	m_focalDistanceMM = packet.getReal("focal-distance-mm", m_focalDistanceMM, DataTreatment::REQUIRED());
}

SdlTypeInfo ThinLensCamera::ciTypeInfo()
{
	return SdlTypeInfo(ETypeCategory::REF_CAMERA, "thin-lens");
}

ExitStatus ThinLensCamera::ciExecute(const std::shared_ptr<ThinLensCamera>& targetResource, const std::string& functionName, const InputPacket& packet)
{
	return ExitStatus::UNSUPPORTED();
}

}// end namespace ph