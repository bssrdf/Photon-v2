#pragma once

#include "Common/primitive_type.h"
#include "PostProcess/Frame.h"

#include <vector>

namespace ph
{

class HdrFrame : public Frame
{
public:
	HdrFrame();
	HdrFrame(const uint32 widthPx, const uint32 heightPx);
	HdrFrame(const HdrFrame& other);
	HdrFrame(HdrFrame&& other);
	virtual ~HdrFrame() override;

	virtual void resize(uint32 newWidthPx, uint32 newHeightPx) override;
	virtual void getPixel(uint32 x, uint32 y, Vector3R* out_pixel) const override;
	virtual void setPixel(uint32 x, uint32 y, real r, real g, real b) override;

	virtual inline uint32 getWidthPx() const override
	{
		return m_widthPx;
	}

	virtual inline uint32 getHeightPx() const override
	{
		return m_heightPx;
	}

	virtual inline uint32 numPixelComponents() const override
	{
		return 3;
	}

	virtual inline const real* getPixelData() const override
	{
		return m_pixelData.data();
	}

	/*inline real getPixel(const uint32 x, const uint32 y) const
	{
		return m_pixelData[y * getWidthPx() + x];
	}*/

	HdrFrame& operator = (const HdrFrame& rhs);
	HdrFrame& operator = (HdrFrame&& rhs);

private:
	uint32 m_widthPx;
	uint32 m_heightPx;

	std::vector<real> m_pixelData;
};

}// end namespace ph