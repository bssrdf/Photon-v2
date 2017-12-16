#pragma once

#include "Common/primitive_type.h"
#include "Math/math_fwd.h"

#include <vector>
#include <cstddef>

namespace ph
{

template<typename ComponentType>
class TFrame final
{
public:
	inline TFrame();
	inline TFrame(uint32 wPx, uint32 hPx);
	inline TFrame(const TFrame& other);
	inline TFrame(TFrame&& other);
	inline ~TFrame() = default;

	inline void getPixel(uint32 x, uint32 y, TVector3<ComponentType>* out_pixel) const;
	inline void setPixel(uint32 x, uint32 y, const TVector3<ComponentType>& pixel);
	inline const ComponentType* getPixelData() const;

	inline TFrame& operator = (TFrame rhs);
	inline TFrame& operator = (TFrame&& rhs);

	inline uint32 widthPx() const
	{
		return m_widthPx;
	}

	inline uint32 heightPx() const
	{
		return m_heightPx;
	}

	inline std::size_t numRgbDataElements() const
	{
		return m_pixelData.size();
	}

	friend inline void swap(TFrame& first, TFrame& second);

private:
	uint32 m_widthPx;
	uint32 m_heightPx;

	std::vector<ComponentType> m_pixelData;

	inline std::size_t calcPixelDataBaseIndex(uint32 x, uint32 y) const;
};

}// end namespace ph

#include "Frame/TFrame.ipp"