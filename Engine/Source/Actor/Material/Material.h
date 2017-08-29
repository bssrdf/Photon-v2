#pragma once

#include "Core/SurfaceBehavior/SurfaceBehavior.h"
#include "FileIO/SDL/TCommandInterface.h"

namespace ph
{

class SurfaceBehavior;
class BSDFcos;
class InputPacket;

class Material : public TCommandInterface<Material>
{
public:
	Material();
	virtual ~Material() = 0;

	virtual void populateSurfaceBehavior(SurfaceBehavior* out_surfaceBehavior) const = 0;

// command interface
public:
	static SdlTypeInfo ciTypeInfo();
	static void ciRegister(CommandRegister& cmdRegister);
};

}// end namespace ph