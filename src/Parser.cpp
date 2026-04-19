#include "../include/Parser.h"

bool Parser::parse(boost::asio::streambuf& buf, std::vector<std::string>& out_args)
{
	if (buf.size() == 0) return false;

	std::istream is(&buf);

	try
	{
		char type;
		if (!(is >> type)) return false;

		if (type != '*') return false;

		int num_elements;
		if (!(is >> num_elements)) return false;
		is.ignore(2);

		for (int i = 0; i < num_elements; ++i) {
			char data_type;
			if (!(is >> data_type)) return false;

			int length;
			if (!(is >> length)) return false;
			is.ignore(2);

			std::string data(length, ' ');
			is.read(&data[0], length);
			is.ignore(2);

			out_args.push_back(data);
		}

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
