#include "Api/ApiDatabase.h"

namespace ph
{

void ApiDatabase::clear()
{
	std::lock_guard<std::mutex> lock(MUTEX());

	RESOURCES<Engine>().removeAll();
	RESOURCES<HdrRgbFrame>().removeAll();
}

}// end namespace ph