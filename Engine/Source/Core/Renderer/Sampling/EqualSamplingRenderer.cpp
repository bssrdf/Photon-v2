#include "Core/Renderer/Sampling/EqualSamplingRenderer.h"
#include "Common/primitive_type.h"
#include "Core/Filmic/Film.h"
#include "World/VisualWorld.h"
#include "Core/Camera/Camera.h"
#include "Core/Ray.h"
#include "Math/constant.h"
#include "Core/SampleGenerator/SampleGenerator.h"
#include "FileIO/SDL/SdlResourcePack.h"
#include "Core/Filmic/HdrRgbFilm.h"
#include "Core/Renderer/RenderWork.h"
#include "Core/Renderer/RenderWorker.h"
#include "Core/Renderer/RendererProxy.h"
#include "Common/assertion.h"
#include "Core/Filmic/SampleFilters.h"
#include "Core/Estimator/BVPTEstimator.h"
#include "Core/Estimator/BNEEPTEstimator.h"
#include "Core/Estimator/Integrand.h"
#include "Core/Filmic/Vector3Film.h"
#include "Core/Renderer/Region/PlateScheduler.h"
#include "Core/Renderer/Region/StripeScheduler.h"
#include "Core/Renderer/Region/GridScheduler.h"
#include "Utility/FixedSizeThreadPool.h"
#include "Utility/utility.h"
#include "Core/Renderer/Region/SpiralGridScheduler.h"
#include "Core/Renderer/Region/TileScheduler.h"

#include <cmath>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>
#include <utility>

namespace ph
{

void EqualSamplingRenderer::doUpdate(const SdlResourcePack& data)
{
	m_updatedRegions.clear();
	m_totalPaths            = 0;
	m_suppliedFractionBits  = 0;
	m_submittedFractionBits = 0;
	
	m_scene           = &data.visualWorld.getScene();
	m_camera          = data.getCamera().get();
	m_sampleGenerator = data.getSampleGenerator().get();

	const Integrand integrand(m_scene, m_camera);
	
	m_estimator->setEstimationIndex(0);
	m_estimator->update(integrand);

	m_mainFilm = HdrRgbFilm(
		getRenderWidthPx(), 
		getRenderHeightPx(), 
		getRenderWindowPx(), 
		m_filter);

	m_filmEstimators.resize(numWorkers());
	m_renderWorks.resize(numWorkers());
	for(uint32 workerId = 0; workerId < numWorkers(); ++workerId)
	{
		m_filmEstimators[workerId] = FilmEnergyEstimator(1, integrand, m_filter);
		m_filmEstimators[workerId].addEstimator(m_estimator.get());

		m_renderWorks[workerId] = TCameraSamplingWork<FilmEnergyEstimator>(
			m_camera,
			&m_filmEstimators[workerId]);
	}

	m_scheduler = std::make_unique<SpiralGridScheduler>(
		numWorkers(),
		WorkUnit(Region(getRenderWindowPx()), m_sampleGenerator->numSampleBatches()),
		50);

	/*m_scheduler = std::make_unique<TileScheduler>(
		numWorkers(),
		WorkUnit(Region(getRenderWindowPx()), m_sampleGenerator->numSampleBatches()),
		Vector2S(10, 50));*/
}

void EqualSamplingRenderer::doRender()
{
	FixedSizeThreadPool workers(numWorkers());

	for(uint32 workerId = 0; workerId < numWorkers(); ++workerId)
	{
		workers.queueWork([this, workerId]()
		{
			auto& renderWork = m_renderWorks[workerId];
			auto& filmEstimator = m_filmEstimators[workerId];

			float suppliedFraction = 0.0f;
			float submittedFraction = 0.0f;
			WorkUnit workUnit;
			while(true)
			{
				std::unique_ptr<SampleGenerator> sampleGenerator;
				{
					std::lock_guard<std::mutex> lock(m_rendererMutex);
					
					if(m_scheduler->schedule(&workUnit))
					{
						suppliedFraction = m_scheduler->getScheduledFraction();
					}
					else
					{
						suppliedFraction = 1.0f;
						break;
					}

					const std::size_t spp = workUnit.getDepth();
					sampleGenerator = m_sampleGenerator->genCopied(spp);
				}

				renderWork.onWorkReport([this, workerId]()
				{
					std::lock_guard<std::mutex> lock(m_rendererMutex);

					m_filmEstimators[workerId].mergeFilmTo(0, m_mainFilm);
					m_filmEstimators[workerId].clearFilm(0);

					addUpdatedRegion(m_filmEstimators[workerId].getFilmEffectiveWindowPx(), true);
				});

				filmEstimator.setFilmDimensions(
					TVector2<int64>(getRenderWidthPx(), getRenderHeightPx()),
					workUnit.getRegion());

				renderWork.setFilmDimensions(filmEstimator.getFilmDimensions());
				renderWork.setSampleGenerator(std::move(sampleGenerator));

				m_suppliedFractionBits.store(
					bitwise_cast<float, std::uint32_t>(suppliedFraction),
					std::memory_order_relaxed);

				renderWork.work();

				{
					std::lock_guard<std::mutex> lock(m_rendererMutex);

					m_scheduler->submit(workUnit);
					submittedFraction = m_scheduler->getSubmittedFraction();

					addUpdatedRegion(filmEstimator.getFilmEffectiveWindowPx(), false);
				}

				m_submittedFractionBits.store(
					bitwise_cast<float, std::uint32_t>(submittedFraction),
					std::memory_order_relaxed);

				m_totalPaths.fetch_add(renderWork.asyncGetStatistics().numSamplesTaken, std::memory_order_relaxed);
			}
		});
	}

	workers.waitAllWorks();
}

ERegionStatus EqualSamplingRenderer::asyncPollUpdatedRegion(Region* const out_region)
{
	PH_ASSERT(out_region);

	std::lock_guard<std::mutex> lock(m_rendererMutex);

	if(m_updatedRegions.empty())
	{
		return ERegionStatus::INVALID;
	}

	const UpdatedRegion updatedRegion = m_updatedRegions.front();
	m_updatedRegions.pop_front();

	*out_region = updatedRegion.region;
	if(updatedRegion.isFinished)
	{
		return ERegionStatus::FINISHED;
	}
	else
	{
		return ERegionStatus::UPDATING;
	}
}

// FIXME: Peeking does not need to ensure correctness of the frame.
// If correctness is not guaranteed, develop methods should be reimplemented. 
// (correctness is guaranteed currently)
void EqualSamplingRenderer::asyncPeekRegion(
	HdrRgbFrame&     out_frame, 
	const Region&    region, 
	const EAttribute attribute)
{
	std::lock_guard<std::mutex> lock(m_rendererMutex);

	m_mainFilm.develop(out_frame, region);
}

void EqualSamplingRenderer::develop(HdrRgbFrame& out_frame, const EAttribute attribute)
{
	asyncPeekRegion(out_frame, getRenderWindowPx(), attribute);
}

void EqualSamplingRenderer::addUpdatedRegion(const Region& region, const bool isUpdating)
{
	for(UpdatedRegion& pendingRegion : m_updatedRegions)
	{
		// later added region takes the precedence
		if(pendingRegion.region.equals(region))
		{
			pendingRegion.isFinished = !isUpdating;
			return;
		}
	}

	m_updatedRegions.push_back(UpdatedRegion{region, !isUpdating});
}

RenderState EqualSamplingRenderer::asyncQueryRenderState()
{
	uint64 totalElapsedMs  = 0;
	uint64 totalNumSamples = 0;
	for(auto&& work : m_renderWorks)
	{
		const auto statistics = work.asyncGetStatistics();
		totalElapsedMs  += work.asyncGetProgress().getElapsedMs();
		totalNumSamples += statistics.numSamplesTaken;
	}

	const float32 samplesPerMs = totalElapsedMs != 0 ?
		static_cast<float32>(m_renderWorks.size() * totalNumSamples) / static_cast<float32>(totalElapsedMs) : 0.0f;

	RenderState state;
	state.setIntegerState(0, m_totalPaths.load(std::memory_order_relaxed) / static_cast<std::size_t>(getRenderWindowPx().calcArea()));
	state.setRealState(0, samplesPerMs * 1000);
	return state;
}

RenderProgress EqualSamplingRenderer::asyncQueryRenderProgress()
{
	RenderProgress workerProgress(0, 0, 0);
	{
		for(auto&& work : m_renderWorks)
		{
			workerProgress += work.asyncGetProgress();
		}
	}

	// HACK
	const std::size_t totalWork = 100000000;
	const float suppliedFraction = bitwise_cast<std::uint32_t, float>(m_suppliedFractionBits.load(std::memory_order_relaxed));
	const float submittedFraction = std::max(bitwise_cast<std::uint32_t, float>(m_submittedFractionBits.load(std::memory_order_relaxed)), suppliedFraction);
	const float workingFraction = submittedFraction - suppliedFraction;
	const std::size_t workDone = static_cast<std::size_t>(totalWork * (suppliedFraction + workerProgress.getNormalizedProgress() * workingFraction));
	RenderProgress totalProgress(totalWork, std::min(workDone, totalWork), workerProgress.getElapsedMs());

	return totalProgress;
}

std::string EqualSamplingRenderer::renderStateName(const RenderState::EType type, const std::size_t index) const
{
	PH_ASSERT_LT(index, RenderState::numStates(type));

	if(type == RenderState::EType::INTEGER)
	{
		switch(index)
		{
		case 0:  return "paths/pixel (avg.)";
		default: return "";
		}
	}
	else if(type == RenderState::EType::REAL)
	{
		switch(index)
		{
		case 0:  return "paths/second";
		default: return "";
		}
	}
	else
	{
		return "";
	}
}

// command interface

EqualSamplingRenderer::EqualSamplingRenderer(const InputPacket& packet) :

	SamplingRenderer(packet),

	m_scene          (nullptr),
	m_camera         (nullptr),
	m_sampleGenerator(nullptr),
	m_filter         (SampleFilters::createGaussianFilter()),
	m_mainFilm       (),
	m_scheduler      (nullptr),

	m_estimator     (nullptr),
	m_renderWorks   (),
	m_filmEstimators(),

	m_updatedRegions       (),
	m_rendererMutex        (),
	m_totalPaths           (),
	m_suppliedFractionBits (),
	m_submittedFractionBits()
{
	const std::string filterName = packet.getString("filter-name");
	m_filter = SampleFilters::create(filterName);

	const std::string estimatorName = packet.getString("estimator", "bneept");
	if(estimatorName == "bvpt")
	{
		m_estimator = std::make_unique<BVPTEstimator>();
	}
	else if(estimatorName == "bneept")
	{
		m_estimator = std::make_unique<BNEEPTEstimator>();
	}

	/*const std::string regionSchedulerName = packet.getString("region-scheduler", "bulk");
	if(regionSchedulerName == "bulk")
	{
		m_workScheduler = std::make_unique<PlateScheduler>();
	}*/
}

SdlTypeInfo EqualSamplingRenderer::ciTypeInfo()
{
	return SdlTypeInfo(ETypeCategory::REF_RENDERER, "equal-sampling");
}

void EqualSamplingRenderer::ciRegister(CommandRegister& cmdRegister)
{
	cmdRegister.setLoader(SdlLoader([](const InputPacket& packet)
	{
		return std::make_unique<EqualSamplingRenderer>(packet);
	}));
}

}// end namespace ph