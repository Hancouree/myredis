#include "../include/Config.h"
#include <fstream>
#include <sstream>
#include <algorithm>

Config::Config(const std::string& file)
{
	init(file);
}

void Config::init(const std::string& file)
{
	std::ifstream fin(file);
	if (!fin.is_open()) throw std::runtime_error("Config file not found: " + file);
	
	std::string line;
	while (std::getline(fin, line)) {
		line.erase(0, line.find_first_not_of(" \t"));
		if (line.empty() || line[0] == '#') continue;
		
		std::istringstream is(line);
		std::string key, value;
		if (is >> key) {
			std::getline(is >> std::ws, value);

			size_t commentPos = value.find('#');
			if (commentPos != std::string::npos) {
				value = value.substr(0, commentPos);
			}
			
			size_t last = value.find_last_not_of(" \t\r\n");
			if (last != std::string::npos) {
				value.erase(last + 1);
			}

			std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			m_options[key] = value;
		}
	}
}

std::string Config::getStr(const std::string& key, const std::string& defaultVal)
{
	return m_options.contains(key) ? m_options[key] : defaultVal;
}

int Config::getInt(const std::string& key, int defaultVal)
{
	try
	{
		return m_options.contains(key) ? std::stoi(m_options[key]) : defaultVal;
	}
	catch (...)
	{
		return defaultVal;
	}
}
