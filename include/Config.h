#pragma once
#include <string>
#include <unordered_map>

class Config
{
public:
	Config(const std::string& file);
	std::string getStr(const std::string& key, const std::string& defaultVal);
	int getInt(const std::string& key, int defaultVal);
private:
	void init(const std::string& file);
	std::unordered_map<std::string, std::string> m_options;
};

