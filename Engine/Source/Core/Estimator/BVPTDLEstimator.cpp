#include "Core/Estimator/BVPTDLEstimator.h"
#include "Core/Ray.h"
#include "Core/HitDetail.h"
#include "Core/SurfaceHit.h"
#include "Core/Intersectable/PrimitiveMetadata.h"
#include "Core/SurfaceBehavior/SurfaceBehavior.h"
#include "Core/SurfaceBehavior/SurfaceOptics.h"
#include "Core/Intersectable/Primitive.h"
#include "Core/Emitter/Emitter.h"
#include "Core/SurfaceBehavior/BsdfSample.h"
#include "Core/Quantity/SpectralStrength.h"
#include "Core/LTABuildingBlock/TSurfaceEventDispatcher.h"
#include "Math/TVector3.h"
#include "Core/Quantity/SpectralStrength.h"
#include "Core/Estimator/Integrand.h"

#include <iostream>

namespace ph
{

void BVPTDLEstimator::estimate(
	const Ray&        ray,
	const Integrand&  integrand,
	EnergyEstimation& out_estimation) const
{
	const auto& surfaceEventDispatcher = TSurfaceEventDispatcher<ESaPolicy::DO_NOT_CARE>(&(integrand.getScene()));

	SpectralStrength& accuRadiance = out_estimation[m_estimationIndex].setValues(0);
	SpectralStrength  accuPathWeight(1);
	
	// 0-bounce
	Ray        firstRay;
	SurfaceHit firstHit;
	{
		// backward tracing to light
		firstRay = Ray(ray).reverse();
		firstRay.setMinT(0.0001_r);// HACK: hard-coded number
		firstRay.setMaxT(std::numeric_limits<real>::max());

		if(!surfaceEventDispatcher.traceNextSurface(firstRay, &firstHit))
		{
			return;
		}

		const auto* const      metadata        = firstHit.getDetail().getPrimitive()->getMetadata();
		const SurfaceBehavior& surfaceBehavior = metadata->getSurface();

		if(surfaceBehavior.getEmitter())
		{
			SpectralStrength emittedRadiance;
			surfaceBehavior.getEmitter()->evalEmittedRadiance(firstHit, &emittedRadiance);

			// avoid excessive, negative weight and possible NaNs
			emittedRadiance.clampLocal(0.0_r, 1000000000.0_r);

			accuRadiance.addLocal(emittedRadiance.mul(accuPathWeight));
		}
	}

	// 1-bounce
	Ray        secondRay;
	SurfaceHit secondHit;
	{
		const Vector3R V = firstRay.getDirection().mul(-1.0f);
		const Vector3R N = firstHit.getShadingNormal();

		BsdfSample bsdfSample;
		bsdfSample.inputs.set(firstHit, V);
		if(!surfaceEventDispatcher.doBsdfSample(firstHit, bsdfSample, &secondRay))
		{
			return;
		}

		if(!surfaceEventDispatcher.traceNextSurface(secondRay, &secondHit))
		{
			return;
		}

		const Vector3R         L          = bsdfSample.outputs.L;
		const SpectralStrength pathWeight = bsdfSample.outputs.pdfAppliedBsdf.mul(N.absDot(L));

		accuPathWeight.mulLocal(pathWeight);

		const auto* const      metadata        = secondHit.getDetail().getPrimitive()->getMetadata();
		const SurfaceBehavior& surfaceBehavior = metadata->getSurface();

		if(surfaceBehavior.getEmitter())
		{
			SpectralStrength emittedRadiance;
			surfaceBehavior.getEmitter()->evalEmittedRadiance(secondHit, &emittedRadiance);

			// avoid excessive, negative weight and possible NaNs
			emittedRadiance.clampLocal(0.0_r, 1000000000.0_r);

			accuRadiance.addLocal(emittedRadiance.mul(accuPathWeight));
		}
	}
}

}// end namespace ph