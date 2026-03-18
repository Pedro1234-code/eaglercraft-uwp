#pragma once

std::string ToLower(const std::string& s);
std::string GetFileExtension(const std::string& path);
std::string JoinStrings(const std::vector<std::string> vec, const std::string& sep);
void EnsureTrailingSlash(std::string& str);
