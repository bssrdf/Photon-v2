#pragma once

#include "FileIO/FileSystem/Path.h"
#include "Frame/TFrame.h"

#include <string>

namespace ph
{

class ExrFileWriter
{
public:
	explicit ExrFileWriter(const Path& filePath);

	bool save(const HdrRgbFrame& frame);
	bool saveHighPrecision(const HdrRgbFrame& frame);

	// HACK
	bool save(const HdrRgbFrame& frame, std::string& byteBuffer);

private:
	Path m_filePath;
};

}// end namespace ph
