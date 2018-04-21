#include "Core/Emitter/OmniModulatedEmitter.h"
#include "Common/assertion.h"
#include "Core/Intersectable/PrimitiveMetadata.h"
#include "Core/SurfaceHit.h"
#include "Core/Texture/TSampler.h"
#include "Core/Sample/DirectLightSample.h"

namespace ph
{

OmniModulatedEmitter::OmniModulatedEmitter(std::unique_ptr<Emitter> source) :
	m_source(std::move(source)),
	m_filter(nullptr)
{
	PH_ASSERT(m_source != nullptr);
}

OmniModulatedEmitter::~OmniModulatedEmitter() = default;

void OmniModulatedEmitter::evalEmittedRadiance(const SurfaceHit& X, SpectralStrength* out_radiance) const
{
	PH_ASSERT(m_filter != nullptr);

	m_source->evalEmittedRadiance(X, out_radiance);

	const Vector3R& emitDirection = X.getIncidentRay().getDirection().mul(-1);

	Vector3R uv;
	m_dirToUv.map(emitDirection, &uv);

	const auto& filterValue = TSampler<SpectralStrength>(EQuantity::RAW).sample(*m_filter, uv);
	out_radiance->mulLocal(filterValue);
}

void OmniModulatedEmitter::genDirectSample(DirectLightSample& sample) const
{
	m_source->genDirectSample(sample);

	const Vector3R& emitDirection = sample.targetPos.sub(sample.emitPos);

	Vector3R uv;
	m_dirToUv.map(emitDirection, &uv);

	const auto& filterValue = TSampler<SpectralStrength>(EQuantity::RAW).sample(*m_filter, uv);
	sample.radianceLe.mulLocal(filterValue);
}

void OmniModulatedEmitter::genSensingRay(Ray* out_ray, SpectralStrength* out_Le, Vector3R* out_eN, real* out_pdfA, real* out_pdfW) const
{
	// TODO
	PH_ASSERT_UNREACHABLE_SECTION();
}

real OmniModulatedEmitter::calcDirectSamplePdfW(const Vector3R& targetPos, const Vector3R& emitPos, const Vector3R& emitN, const Primitive* hitPrim) const
{
	const real pdfW = m_source->calcDirectSamplePdfW(targetPos, emitPos, emitN, hitPrim);

	// TODO: if importance sampling is used, pdfW should be changed here

	return pdfW;
}

void OmniModulatedEmitter::setFilter(const std::shared_ptr<TTexture<SpectralStrength>>& filter)
{
	PH_ASSERT(filter != nullptr);

	m_filter = filter;
}

}// end namespace ph