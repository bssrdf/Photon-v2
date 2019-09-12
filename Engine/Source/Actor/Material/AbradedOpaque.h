#pragma once

#include "Actor/Material/SurfaceMaterial.h"
#include "FileIO/SDL/TCommandInterface.h"
#include "Actor/Material/Utility/ConductiveInterfaceInfo.h"
#include "Actor/Material/Utility/MicrosurfaceInfo.h"

namespace ph
{

class AbradedOpaque : public SurfaceMaterial, public TCommandInterface<AbradedOpaque>
{
public:
	AbradedOpaque();

	void genSurface(CookingContext& context, SurfaceBehavior& behavior) const override;

private:
	ConductiveInterfaceInfo m_interfaceInfo;
	MicrosurfaceInfo        m_microsurfaceInfo;

// command interface
public:
	explicit AbradedOpaque(const InputPacket& packet);
	static SdlTypeInfo ciTypeInfo();
	static void ciRegister(CommandRegister& cmdRegister);
};

}// end namespace ph

/*
	<SDL_interface>

	<category>  material          </category>
	<type_name> abraded-opaque    </type_name>
	<extend>    material.material </extend>

	<name> Abraded Opaque </name>
	<description>
		Able to model surfaces ranging from nearly specular to extremely rough
		appearances.
	</description>

	<command type="creator">
		<input name="type" type="string">
			<description>
				Possible value are "iso-metallic-ggx" and "aniso-metallic-ggx",
				for isotropic and anisotropic surface appearances, respectively.
			</description>
		</input>
		<input name="roughness" type="real">
			<description>
				Isotropic surface roughness in [0, 1], the material will appear
				to be smoother with smaller roughness value.
			</description>
		</input>
		<input name="f0" type="vector3">
			<description>
				Surface reflectance on normal incidence. This value is expected
				to be given in linear-SRGB space. When this parameter is used,
				the underlying Fresnel model will be an approximated one which
				is pretty popular in real-time graphics.
			</description>
		</input>
		<input name="roughness-u" type="real">
			<description>
				Similar to the roughness parameter, but is used for anisotropic 
				surface appearances. This value controls the U component of 
				surface roughness.
			</description>
		</input>
		<input name="roughness-v" type="real">
			<description>
				Similar to the roughness parameter, but is used for anisotropic
				surface appearances. This value controls the V component of
				surface roughness.
			</description>
		</input>
	</command>

	</SDL_interface>
*/
