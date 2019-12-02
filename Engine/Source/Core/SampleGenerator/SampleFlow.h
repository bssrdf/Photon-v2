#pragma once

#include "Common/primitive_type.h"
#include "Common/assertion.h"
#include "Math/Random.h"

#include <cstddef>
#include <array>

namespace ph
{

class SampleFlow final
{
public:
	SampleFlow();
	SampleFlow(const real* savedDims, std::size_t numSavedDims);

	template<std::size_t N>
	std::array<real, N> flowND();

private:
	const real* m_savedDims;
	std::size_t m_numSavedDims;
	std::size_t m_numReadDims;
};

// In-header Implementations:

inline SampleFlow::SampleFlow() :
	m_savedDims   (nullptr),
	m_numSavedDims(0),
	m_numReadDims (0)
{}

inline SampleFlow::SampleFlow(const real* const savedDims, const std::size_t numSavedDims) :
	m_savedDims   (savedDims),
	m_numSavedDims(numSavedDims),
	m_numReadDims (0)
{
	PH_ASSERT(savedDims);
}

template<std::size_t N>
inline std::array<real, N> SampleFlow::flowND()
{
	std::array<real, N> sample;
	if(m_savedDims)
	{
		for(std::size_t i = 0; i < N; ++i)
		{
			sample[i] = m_numReadDims < m_numSavedDims ? 
				m_savedDims[m_numReadDims] : math::Random::genUniformReal_i0_e1();

			++m_numReadDims;
		}
	}
	else
	{
		for(std::size_t i = 0; i < N; ++i)
		{
			sample[i] = math::Random::genUniformReal_i0_e1();
		}
	}
	return sample;
}

}// end namespace ph