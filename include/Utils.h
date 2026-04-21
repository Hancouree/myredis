#pragma once
#include <string>
#include <deque>
#include <unordered_map>

namespace Utils {
	std::string integer(int value);
	std::string bulk(const std::string& str);
	std::string list(const std::deque<std::string>& l);
	std::string hash(const std::unordered_map<std::string, std::string>& h);
	std::string emptyArr();
	std::string nil();
	std::string ok();
	std::string error(const std::string& msg);
	std::string pong();
}