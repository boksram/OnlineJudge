#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <boost/algorithm/string.hpp>

const std::string path_name = "./temp/";

namespace Util
{
    class time_util
    {
    public:
        static std::string gettime()
        {
            struct timeval _time;
            gettimeofday(&_time,nullptr);
            return std::to_string(_time.tv_sec);
        }

        static std::string getmillitime()
        {
            struct timeval _time;
            gettimeofday(&_time,nullptr);
            return std::to_string(_time.tv_sec * 1000 + _time.tv_usec / 1000);
        }
    };

    class name_util
    {
    public:
        static std::string addsuffix(const std::string& file_name, const std::string suffix)
        {
            std::string full_name(path_name);
            full_name += file_name;
            full_name += suffix;
            return full_name;
        }

        static std::string src(const std::string &file_name)
        {
            return addsuffix(file_name,".cpp");
        }

        static std::string exe(const std::string &file_name)
        {
            return addsuffix(file_name,".exe");
        }

        static std::string compile_error(const std::string &file_name)
        {
            return addsuffix(file_name,".compile_err");
        }

        static std::string std_in(const std::string &file_name)
        {
            return addsuffix(file_name,".stdin");
        }

        static std::string std_out(const std::string &file_name)
        {
            return addsuffix(file_name,".stdout");
        }

        static std::string std_err(const std::string &file_name)
        {
            return addsuffix(file_name,".stderr");
        }
    };

    class file_util
    {
    public:
        static bool isfileexist(const std::string &path_name)
        {
            struct stat st;
            if (stat(path_name.c_str(), &st) == 0)
                return true;
            return false;
        }

        static std::string uniqfilename()
        {
            static std::atomic_uint id(0);
            id++;
            std::string ms = time_util::getmillitime();
            std::string unique_id = std::to_string(id);
            return ms + "_" + unique_id;
        }

        static bool writefile(const std::string &target, const std::string &content)
        {
            std::ofstream out(target);
            if(!out.is_open())
            {
                return false;
            }
            out.write(content.c_str(), content.size());
            out.close();
            return true;
        }

        static bool readfile(const std::string &target, std::string *content, bool keep = false)
        {
            (*content).clear();
            std::ifstream in(target);
            if(!in.is_open()) return false;
            std::string line;
            while (std::getline(in, line))
            {
                (*content)+=line;
                (*content) += (keep ? "\n" : "");
            }
            in.close();
            return true;
        }
    };

    class string_util
    {
        public:
            static void splitstring(const std::string &str, std::vector<std::string> *target, const std::string &sep)
            {
                boost::split((*target),str,boost::is_any_of(sep),boost::algorithm::token_compress_on);
            }
    };

}
