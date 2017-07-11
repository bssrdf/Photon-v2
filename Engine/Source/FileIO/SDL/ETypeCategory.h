#pragma once

namespace ph
{

enum class ETypeCategory
{
	// world command related types
	PRIMITIVE, 
	REF_GEOMETRY, 
	REF_MATERIAL,
	REF_MOTION,
	REF_LIGHT_SOURCE, 
	REF_ACTOR, 

	// core command related types
	REF_CAMERA,
	REF_FILM,
	REF_INTEGRATOR,
	REF_SAMPLER,
	
	// special values
	MIN         = PRIMITIVE,
	MAX         = REF_SAMPLER,
	UNSPECIFIED = MAX + 1
};

}// end namespace ph