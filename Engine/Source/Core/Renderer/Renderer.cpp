#include "Core/Renderer/Renderer.h"
#include "Common/primitive_type.h"
#include "Core/Filmic/Film.h"
#include "World/VisualWorld.h"
#include "Core/Camera/Camera.h"
#include "Core/Ray.h"
#include "Math/constant.h"
#include "Core/SampleGenerator/SampleGenerator.h"
#include "Core/Filmic/HdrRgbFilm.h"
#include "Core/Renderer/RenderWorker.h"
#include "Core/Renderer/RendererProxy.h"
#include "Common/Logger.h"
#include "DataIO/SDL/InputPacket.h"
#include "DataIO/SDL/DataTreatment.h"
#include "DataIO/SDL/SdlResourcePack.h"

#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <utility>

namespace ph
{

namespace
{
	const Logger logger(LogSender("Renderer"));
}

Renderer::~Renderer() = default;

void Renderer::update(const SdlResourcePack& data)
{
	logger.log("# render workers = " + std::to_string(numWorkers()));

	const auto resolution = data.getCamera()->getResolution();
	m_widthPx = resolution.x;
	m_heightPx = resolution.y;

	// TODO: render region is the intersection of crop window and resolution
	// Nothing is cropped if no suitable window is present
	if(m_cropWindowPx.isValid() && m_cropWindowPx.getArea() == 0)
	{
		m_cropWindowPx = TAABB2D<int64>(
			{0, 0}, 
			{static_cast<int64>(resolution.x), static_cast<int64>(resolution.y)});
	}
	logger.log("render region = " + getCropWindowPx().toString());

	logger.log("updating...");

	Timer updateTimer;
	updateTimer.start();
	m_isUpdating.store(true, std::memory_order_relaxed);// FIXME: seq_cst

	doUpdate(data);

	m_isUpdating.store(false, std::memory_order_relaxed);
	updateTimer.finish();

	logger.log("update time: " + std::to_string(updateTimer.getDeltaMs()) + " ms");
}

void Renderer::render()
{
	logger.log("rendering...");

	Timer renderTimer;
	renderTimer.start();
	m_isRendering.store(true, std::memory_order_relaxed);

	doRender();

	m_isRendering.store(false, std::memory_order_relaxed);
	renderTimer.finish();

	logger.log("render time: " + std::to_string(renderTimer.getDeltaMs()) + " ms");
}

void Renderer::setNumWorkers(const uint32 numWorkers)
{
	m_numWorkers = numWorkers;

	/*m_workers.resize(numThreads);
	for(uint32 ti = 0; ti < numThreads; ti++)
	{
		m_workers[ti] = RenderWorker(RendererProxy(this), ti);
	}*/
}

// FIXME: without synchronizing, other threads may never observe m_workers being changed
//void Renderer::asyncQueryStatistics(float32* const out_percentageProgress, 
//                                    float32* const out_samplesPerSecond)
//{
//	uint64  totalWork     = 0;
//	uint64  totalWorkDone = 0;
//	for(auto& worker : m_workers)
//	{
//		const auto progress = worker.asyncQueryProgress();
//
//		// FIXME: this calculation can be wrong if there are more works than workers
//		totalWork     += progress.totalWork;
//		totalWorkDone += progress.workDone;
//	}
//
//	*out_percentageProgress = totalWork != 0 ? 
//		static_cast<float32>(totalWorkDone) / static_cast<float32>(totalWork) * 100.0f : 0.0f;
//
//	// HACK
//	const auto states = asyncQueryRenderStates();
//	*out_samplesPerSecond = static_cast<float32>(states.fltStates[0]) * 1000.0f;
//}

// command interface

Renderer::Renderer(const InputPacket& packet) : 
	m_widthPx(0),
	m_heightPx(0),
	m_cropWindowPx({0, 0}, {0, 0}),
	m_isUpdating(false),
	m_isRendering(false)
{
	setNumWorkers(1);

	/*const integer filmWidth  = packet.getInteger("width",  1280, DataTreatment::REQUIRED());
	const integer filmHeight = packet.getInteger("height", 720,  DataTreatment::REQUIRED());*/
	const integer rectX      = packet.getInteger("rect-x", m_cropWindowPx.minVertex.x);
	const integer rectY      = packet.getInteger("rect-y", m_cropWindowPx.minVertex.y);
	const integer rectW      = packet.getInteger("rect-w", m_cropWindowPx.getWidth());
	const integer rectH      = packet.getInteger("rect-h", m_cropWindowPx.getHeight());

	/*m_widthPx      = filmWidth;
	m_heightPx     = filmHeight;*/
	m_cropWindowPx = TAABB2D<int64>({rectX, rectY}, {rectX + rectW, rectY + rectH});
}

SdlTypeInfo Renderer::ciTypeInfo()
{
	return SdlTypeInfo(ETypeCategory::REF_RENDERER, "renderer");
}

void Renderer::ciRegister(CommandRegister& cmdRegister)
{}

}// end namespace ph
