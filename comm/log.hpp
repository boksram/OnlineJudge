#pragma once
#include <iostream>
#include <string>
#include "util.hpp"

namespace Log
{
#define LOG(level) log(#level, __FILE__, __LINE__)
    enum
    {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        FATAL
    };

    inline std::ostream &log(const std::string &level, const std::string &file_name, int line)
    {
        std::string message = "[";
        message += level;
        message += ']';

        message += "[";
        message += file_name;
        message += "]";

        message += "[";
        message += std::to_string(line);
        message += ']';

        message += "[";
        message += Util::time_util::gettime();
        message += ']';

        std::cout << message;

        return std::cout;
    }
}