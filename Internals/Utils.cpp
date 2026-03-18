
#include "pch.h"

#include "Utils.h"

std::string ToLower(const std::string& s)
{
	std::string temp(s);
	std::transform(temp.begin(), temp.end(), temp.begin(), [](char c) { return static_cast<char>(tolower(c)); });
	return temp;
}

std::string GetFileExtension(const std::string& path)
{
	size_t i = path.find_last_of('.');

	if (i == std::string::npos)
		return std::string();
	else
		return path.substr(i);
}

std::string JoinStrings(const std::vector<std::string> vec, const std::string& sep)
{
	std::ostringstream ss;
	for (auto i = vec.begin(), end = vec.end(); i != end; ++i)
	{
		ss << *i;

		if (i + 1 != end)
			ss << sep;
	}
	return ss.str();
}

void EnsureTrailingSlash(std::string& str)
{
	if (str.empty())
		return;

	char last = str.at(str.length() - 1);
	if (last != '/' && last != '\\')
		str += "\\";
}
