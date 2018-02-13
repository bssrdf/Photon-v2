#pragma once

#include "Common/primitive_type.h"
#include "Math/TVector3.h"
#include "FileIO/SDL/TCommandInterface.h"
#include "Core/Texture/TTexture.h"
#include "Core/Quantity/SpectralStrength.h"
#include "Core/Texture/TConstantTexture.h"
#include "Actor/Image/ImageParameter.h"

#include <iostream>
#include <memory>

namespace ph
{

class CookingContext;

class Image : public TCommandInterface<Image>
{
public:
	Image();
	Image(const ImageParameter& param);
	virtual ~Image() = 0;

	virtual std::shared_ptr<TTexture<real>> genTextureReal(
		CookingContext& context) const;

	virtual std::shared_ptr<TTexture<Vector3R>> genTextureVector3R(
		CookingContext& context) const;

	virtual std::shared_ptr<TTexture<SpectralStrength>> genTextureSpectral(
		CookingContext& context) const;

	ImageParameter getParameter() const;

private:
	ImageParameter m_param;

	template<typename OutputType>
	inline std::shared_ptr<TTexture<OutputType>> genDefaultTexture() const
	{
		std::cerr << "warning: at Image::genTexture(), "
	              << "no implementation provided, generating a constant one" << std::endl;

		return std::make_shared<TConstantTexture<OutputType>>(OutputType(1));
	}

// command interface
public:
	static SdlTypeInfo ciTypeInfo();
	static void ciRegister(CommandRegister& cmdRegister);
	static void ciRegisterMathFunctions(CommandRegister& cmdRegister);
};

}// end namespace ph

#include "Actor/Image/Image.ipp"