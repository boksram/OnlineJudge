#pragma once
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../comm/log.hpp"
#include "../comm/util.hpp"
#include <sys/time.h>
#include <sys/resource.h>

namespace Runner
{
    using namespace Util;
    using namespace Log;
    class runner
    {
    public:
        runner(){}
        ~runner(){}

        static void setproclimit(int _cpu_limit, int _mem_limit)
        {
            struct rlimit cpu_rlimit;
            cpu_rlimit.rlim_max = RLIM_INFINITY;
            cpu_rlimit.rlim_cur = _cpu_limit;
            setrlimit(RLIMIT_CPU,&cpu_rlimit);

            struct rlimit mem_rlimit;
            mem_rlimit.rlim_max = RLIM_INFINITY;
            mem_rlimit.rlim_cur = _mem_limit*1024;
            setrlimit(RLIMIT_AS,&mem_rlimit);
        }

        static int run(const std::string &file_name, int cpu_limit, int mem_limit)
        {
            std::string _execute = name_util::exe(file_name);
            std::string _stdin = name_util::std_in(file_name);
            std::string _stdout = name_util::std_out(file_name);
            std::string _stderror = name_util::std_err(file_name);
            umask(0);
            int _in = open(_stdin.c_str(), O_CREAT | O_RDONLY, 0644);
            int _out = open(_stdout.c_str(), O_CREAT | O_WRONLY, 0644);
            int _error = open(_stderror.c_str(), O_CREAT | O_WRONLY, 0644);
            if (_in < 0 || _out < 0 || _error < 0)
            {
                LOG(ERROR) << "Failed to open standard files" << "\n";
                return -1;
            }

            pid_t id =fork();
            if (id < 0)
            {
                LOG(ERROR) << "Failed to creat child process while running" << "\n";
                close(_in);
                close(_out);
                close(_error);
                return -2;
            }
            else if(id == 0)
            {
                dup2(_in, 0);
                dup2(_out, 1);
                dup2(_error, 2);

                setproclimit(cpu_limit, mem_limit);
                execl(_execute.c_str(), _execute.c_str(),nullptr);
                exit(1);
            }
            else
            {
                close(_in);
                close(_out);
                close(_error);
                int status = 0;
                waitpid(id, &status, 0);
                LOG(INFO) << "Run done,info:" << (status & 0X7F) << "\n";
                return status & 0X7F;
            }
        }
    };
}
