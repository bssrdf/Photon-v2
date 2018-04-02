#include <FileIO/Data/IesFile.h>

#include <gtest/gtest.h>

TEST(IesFileLoadingTest, Parse1995Files)
{
	using namespace ph;

	const double acceptableError = 0.0001;

	///////////////////////////////////////////////////////////////////////////
	// file 1

	IesFile file1(Path("./Resource/Test/IES/1.ies"));
	ASSERT_TRUE(file1.load());
	
	EXPECT_TRUE(file1.getIesFileType() == IesFile::EIesFileType::LM_63_1995);
	EXPECT_STREQ(file1.getTestInfo().c_str(), " 3310_1 BY: ERCO / LUM650");
	EXPECT_STREQ(file1.getLuminaireManufacturer().c_str(), " ERCO Leuchten GmbH");
	EXPECT_STREQ(file1.getLuminaireCatalogNumber().c_str(), " 83479000");
	EXPECT_STREQ(file1.getLuminaireDescription().c_str(), " Panarc Corridor luminaire");
	EXPECT_STREQ(file1.getLampCatalogNumber().c_str(), " TC-F 36W");
	EXPECT_EQ(file1.getNumLamps(), 1);
	EXPECT_NEAR(file1.getLumensPerLamp(), 2700.0_r, acceptableError);
	EXPECT_NEAR(file1.getCandelaMultiplier(), 1.0_r, acceptableError);
	EXPECT_EQ(file1.getNumVerticalAngles(), 19);
	EXPECT_EQ(file1.getNumHorizontalAngles(), 7);

	const auto verticalAngles1   = file1.getVerticalAngles();
	const auto horizontalAngles1 = file1.getHorizontalAngles();
	const auto candelaValues1    = file1.getCandelaValues();
	ASSERT_EQ(verticalAngles1.size(),   file1.getNumVerticalAngles());
	ASSERT_EQ(horizontalAngles1.size(), file1.getNumHorizontalAngles());
	ASSERT_EQ(candelaValues1.size(),    file1.getNumHorizontalAngles());
	for(const auto& verticalValues : candelaValues1)
	{
		ASSERT_EQ(verticalValues.size(), file1.getNumVerticalAngles());
	}

	EXPECT_NEAR(candelaValues1[0][0], 197.8, acceptableError);
	EXPECT_NEAR(candelaValues1[0][1], 213.8, acceptableError);
	EXPECT_NEAR(candelaValues1[0][2], 275.2, acceptableError);
	EXPECT_NEAR(candelaValues1[0].back(), 1.1, acceptableError);

	EXPECT_NEAR(candelaValues1[2][0], 197.8, acceptableError);
	EXPECT_NEAR(candelaValues1[2][2], 250.0, acceptableError);
	EXPECT_NEAR(candelaValues1[2][5], 412.3, acceptableError);
	EXPECT_NEAR(candelaValues1[2].back(), 1.1, acceptableError);

	EXPECT_NEAR(candelaValues1.back()[0], 197.8, acceptableError);
	EXPECT_NEAR(candelaValues1.back()[1], 197.4, acceptableError);
	EXPECT_NEAR(candelaValues1.back()[3], 193.2, acceptableError);
	EXPECT_NEAR(candelaValues1.back().back(), 0.0, acceptableError);
}