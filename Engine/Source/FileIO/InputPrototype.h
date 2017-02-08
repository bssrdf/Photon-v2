#pragma once

#include <vector>
#include <utility>
#include <string>

namespace ph
{

class InputPrototype final
{
public:
	std::vector<std::pair<std::string, std::string>> typeNamePairs;

	void addInteger(const std::string& name);
	void addReal(const std::string& name);
	void addString(const std::string& name);
	void addVector3r(const std::string& name);
	void addQuaternionR(const std::string& name);

	std::string toString() const;
};

}// namespace ph