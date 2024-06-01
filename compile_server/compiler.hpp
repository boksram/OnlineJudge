#pragma once
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../comm/util.hpp"
#include "../comm/log.hpp"

namespace Compiler
{
    using namespace Util;
    class compiler
    {
    public:
        compiler()
        {}

        ~compiler()
        {}

        static bool compile(const std::string &file_name)
        {
            pid_t id = fork();
            if (id < 0)
            {
                Log::LOG(ERROR) << "Failed to create the child process due to an internal error" << "\n";
                return false;
            }
            else if (id == 0)
            {
                umask(0);
                int _error = open(name_util::compile_error(file_name).c_str(), O_CREAT | O_WRONLY, 0644);
                if (_error < 0)
                {
                    Log::LOG(WARNING) << "Failed to open the error file" << "\n";
                    exit(1);
                }
                dup2(_error,2);

                execlp("g++", "g++", name_util::src(file_name).c_str(), "-o", name_util::exe(file_name).c_str(), "-std=c++11", nullptr);
                Log::LOG(ERROR) << "Failed to start g++ compiler, maybe the parameter passed in is incorrect" << "\n";
                exit(2);
            }
            else
            {
                waitpid(id, nullptr, 0);
                if (Util::file_util::isfileexist(name_util::exe(file_name)))
                {
                    Log::LOG(INFO) << name_util::src(file_name) << " compile success!" << "\n";
                    return true;
                }
                Log::LOG(ERROR) << "Compilation failed. No executable program was formed" << "\n";
                return false;
            }
        }
    };
}
