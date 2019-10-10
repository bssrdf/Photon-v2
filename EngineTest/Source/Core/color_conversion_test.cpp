#include <Core/Quantity/ColorSpace.h>
#include <Core/Quantity/SpectralStrength.h>
#include <Math/TVector3.h>

#include <gtest/gtest.h>

using namespace ph;
using namespace ph::math;

TEST(ColorConversionTest, SrgbCieXyzInterConversion)
{
	const real ACCEPTABLE_ERROR = 0.0003_r;

	Vector3R color;

	color = ColorSpace::sRGB_to_linear_sRGB({0, 0, 0});
	EXPECT_NEAR(color.x, 0, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.y, 0, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.z, 0, ACCEPTABLE_ERROR);

	color = ColorSpace::linear_sRGB_to_sRGB({0, 0, 0});
	EXPECT_NEAR(color.x, 0, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.y, 0, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.z, 0, ACCEPTABLE_ERROR);

	color = ColorSpace::sRGB_to_linear_sRGB({1, 1, 1});
	EXPECT_NEAR(color.x, 1, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.y, 1, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.z, 1, ACCEPTABLE_ERROR);

	color = ColorSpace::linear_sRGB_to_sRGB({1, 1, 1});
	EXPECT_NEAR(color.x, 1, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.y, 1, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.z, 1, ACCEPTABLE_ERROR);

	color = ColorSpace::CIE_XYZ_D65_to_linear_sRGB({0, 0, 0});
	EXPECT_NEAR(color.x, 0, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.y, 0, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.z, 0, ACCEPTABLE_ERROR);

	color = ColorSpace::linear_sRGB_to_CIE_XYZ_D65({0, 0, 0});
	EXPECT_NEAR(color.x, 0, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.y, 0, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.z, 0, ACCEPTABLE_ERROR);

	Vector3R normalizedD65_XYZ(0.95047_r, 1.0_r, 1.08883_r);

	color = ColorSpace::CIE_XYZ_D65_to_linear_sRGB(normalizedD65_XYZ);
	EXPECT_NEAR(color.x, 1, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.y, 1, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.z, 1, ACCEPTABLE_ERROR);

	color = ColorSpace::linear_sRGB_to_CIE_XYZ_D65({1, 1, 1});
	EXPECT_NEAR(color.x, normalizedD65_XYZ.x, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.y, normalizedD65_XYZ.y, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.z, normalizedD65_XYZ.z, ACCEPTABLE_ERROR);
}

TEST(ColorConversionTest, SpectrumToCieXyzConversion)
{
	const real ACCEPTABLE_ERROR = 0.0003_r;

	Vector3R color;

	color = ColorSpace::SPD_to_CIE_XYZ_D65<ESourceHint::ILLUMINANT>(ColorSpace::get_D65_SPD());
	EXPECT_NEAR(color.x, 0.95047_r, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.y, 1.00000_r, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.z, 1.08883_r, ACCEPTABLE_ERROR);

	color = ColorSpace::SPD_to_CIE_XYZ_D65<ESourceHint::ILLUMINANT>(SampledSpectralStrength(0));
	EXPECT_NEAR(color.x, 0, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.y, 0, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.z, 0, ACCEPTABLE_ERROR);

	color = ColorSpace::SPD_to_CIE_XYZ_E<ESourceHint::ILLUMINANT>(ColorSpace::get_E_SPD());
	EXPECT_NEAR(color.x, 1.0_r, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.y, 1.0_r, ACCEPTABLE_ERROR);
	EXPECT_NEAR(color.z, 1.0_r, ACCEPTABLE_ERROR);
}
