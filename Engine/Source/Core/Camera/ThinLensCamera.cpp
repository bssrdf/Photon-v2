#include "Core/Camera/ThinLensCamera.h"
#include "Core/Sample.h"
#include "Core/Ray.h"
#include "Math/Transform/Transform.h"
#include "FileIO/SDL/InputPacket.h"
#include "Math/Random.h"
#include "Common/assertion.h"

#include <iostream>

namespace ph
{

void ThinLensCamera::genSensedRay(const math::Vector2R& filmNdcPos, Ray* const out_ray) const
{
	math::Vector3R filmPosMM;
	m_filmToCamera->transformP(math::Vector3R(filmNdcPos.x, filmNdcPos.y, 0), &filmPosMM);

	// subtracting lens' center position is omitted since it is at (0, 0, 0) mm
	const math::Vector3R lensCenterToFilmDir = filmPosMM.normalize();

	PH_ASSERT_GT(lensCenterToFilmDir.z, 0);
	const real           focalPlaneDistMM = m_focalDistanceMM / lensCenterToFilmDir.z;
	const math::Vector3R focalPlanePosMM  = lensCenterToFilmDir.mul(-focalPlaneDistMM);

	math::Vector3R lensPosMM;
	lensPosMM.z = 0;
	genRandomSampleOnDisk(m_lensRadiusMM, &lensPosMM.x, &lensPosMM.y);

	math::Vector3R worldLensPos;
	m_cameraToWorld->transformP(lensPosMM, &worldLensPos);

	// XXX: numerical error can be large when focalPlanePosMM is far away
	math::Vector3R worldSensedRayDir;
	m_cameraToWorld->transformV(lensPosMM - focalPlanePosMM, &worldSensedRayDir);
	worldSensedRayDir.normalizeLocal();

	PH_ASSERT(out_ray);
	out_ray->setDirection(worldSensedRayDir);
	out_ray->setOrigin(worldLensPos);
	out_ray->setMinT(0.0001_r);// HACK: hard-coded number
	out_ray->setMaxT(std::numeric_limits<real>::max());
}

void ThinLensCamera::evalEmittedImportanceAndPdfW(
	const math::Vector3R& targetPos,
	math::Vector2R* const out_filmCoord,
	math::Vector3R* const out_importance,
	real* out_filmArea,
	real* const out_pdfW) const
{
	std::cerr << "ThinLensCamera::evalEmittedImportanceAndPdfW() not implemented" << std::endl;
}

void ThinLensCamera::genRandomSampleOnDisk(const real radius, real* const out_x, real* const out_y)
{
	const real r   = radius * std::sqrt(math::Random::genUniformReal_i0_e1());
	const real phi = math::constant::two_pi<real> * math::Random::genUniformReal_i0_e1();
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

void ThinLensCamera::ciRegister(CommandRegister& cmdRegister)
{
	cmdRegister.setLoader(
		SdlLoader([](const InputPacket& packet)
		{
			return std::make_unique<ThinLensCamera>(packet);
		}));
}

}// end namespace ph
