#pragma once
#include <boost/asio.hpp>

class Parser {
public:
	bool parse(boost::asio::streambuf& buf, std::vector<std::string>& out_args);
};
