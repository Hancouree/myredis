#include "../include/Parser.h"
#include <boost/asio/buffers_iterator.hpp>
#include <string>
#include <vector>
#include <iterator>

bool Parser::parse(boost::asio::streambuf& buf, std::vector<std::string>& out_args)
{
    if (buf.size() == 0) return false;

    auto buffers = buf.data();
    auto begin = boost::asio::buffers_begin(buffers);
    auto end = boost::asio::buffers_end(buffers);
    auto it = begin;

    out_args.clear();

    try
    {
        if (it == end) return false;

        char type = *it;
        ++it;
        if (type != '*') return false;

        std::string num_elements_str;
        if (!readLine(it, end, num_elements_str)) return false;

        int num_elements = 0;
        try
        {
            num_elements = std::stoi(num_elements_str);
        }
        catch (...)
        {
            return false;
        }

        if (num_elements < 0 || num_elements > 1024) return false;

        for (int i = 0; i < num_elements; ++i) {
            if (it == end) return false;

            char data_type = *it;
            ++it;
            if (data_type != '$') return false;

            std::string length_str;
            if (!readLine(it, end, length_str)) return false;

            int length = 0;
            try
            {
                length = std::stoi(length_str);
            }
            catch (...)
            {
                return false;
            }

            if (length == -1) {
                out_args.push_back("");
                continue;
            }

            if (length < -1 || length > 512 * 1024 * 1024) return false;
            if (end - it < length + 2) {
                return false; 
            }

            std::string data(it, it + length);
            it += length;

            if (*it != '\r') return false;
            ++it;
            if (*it != '\n') return false;
            ++it;

            out_args.push_back(std::move(data));
        }

        size_t bytesConsumed = std::distance(begin, it);
        buf.consume(bytesConsumed);

        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool Parser::readLine(const_iterator& it, const const_iterator& end, std::string& line)
{
    auto temp_it = it;
    line.clear();
    while (temp_it != end) {
        char c = *temp_it;
        ++temp_it;
        if (c == '\r') {
            if (temp_it != end && *temp_it == '\n') {
                it = ++temp_it; 
                return true;
            }
            return false;
        }
        line += c;
    }
    return false; 
}