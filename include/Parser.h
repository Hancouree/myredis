#pragma once
#include <boost/asio.hpp>
#include <boost/asio/buffers_iterator.hpp>

class Parser {
public:
	using const_buffers_type = boost::asio::streambuf::const_buffers_type;
	using const_iterator = boost::asio::buffers_iterator<const_buffers_type>;

	bool parse(boost::asio::streambuf& buf, std::vector<std::string>& out_args);

private:
	bool readLine(const_iterator& it, const const_iterator& end, std::string& line);
};
