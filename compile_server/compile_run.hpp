#pragma once
#include "compiler.hpp"
#include "runner.hpp"
#include <json/json.h>
#include <signal.h>
#include <unistd.h>
#include "../comm/log.hpp"
#include "../comm/util.hpp"

namespace CompileAndRun
{
    using namespace Log;
    using namespace Util;
    using namespace Compiler;
    using namespace Runner;
    class compileandrun
    {
    public:

        static std::string codetodesc(int code, const std::string file_name)
        {
            std::string desc;
            switch(code)
            {
                case 0:
                    desc = "Both compile and run are success";
                    break;
                case -1:
                    desc = "The code user commited is null";
                    break;
                case -2:
                    desc = "Unknown error";
                    break;
                case -3:
                    // desc = "Compile error";
                    file_util::readfile(name_util::compile_error(file_name), &desc, true);
                    break;
                case SIGABRT:
                    desc = "Exceed resource limit";
                    break;
                case SIGXCPU:
                    desc = "Exceed time limit";
                    break;
                case SIGFPE:
                    desc = "float overflow";
                    break;
                default:
                    desc = "Unknown error";
                    break;
            }
            return desc;
        }

        static void removetempfile(const std::string &file_name)
        {
            std::string _src = name_util::src(file_name);
            if(file_util::isfileexist(_src)) unlink(_src.c_str());

            std::string _compile_error = name_util::compile_error(file_name);
            if(file_util::isfileexist(_compile_error)) unlink(_compile_error.c_str());

            std::string _exe = name_util::exe(file_name);
            if(file_util::isfileexist(_exe)) unlink(_exe.c_str());

            std::string _stdin = name_util::std_in(file_name);
            if(file_util::isfileexist(_stdin)) unlink(_stdin.c_str());

            std::string _stdout = name_util::std_out(file_name);
            if(file_util::isfileexist(_stdout)) unlink(_stdout.c_str());

            std::string _stderr = name_util::std_err(file_name);
            if(file_util::isfileexist(_stderr)) unlink(_stderr.c_str());

        }

        static void start(const std::string &in_json, std::string *out_json)
        {
            Json::Value in_value;
            Json::Reader reader;
            reader.parse(in_json,in_value);
            std::string code = in_value["code"].asString();
            std::string input = in_value["input"].asString();
            int cpu_limit = in_value["cpu_limit"].asInt();
            int mem_limit = in_value["mem_limit"].asInt();

            Json::Value out_value;
            int statuscode = 0;
            int runresult = 0;
            std::string file_name;
            if(code.size()==0)
            {
                statuscode = -1;
                goto End;
            }

            file_name = file_util::uniqfilename();
            if(!file_util::writefile(name_util::src(file_name),code))
            {
                statuscode = -2;
                goto End;
            }

            if(!compiler::compile(file_name))
            {
                statuscode = -3;
                goto End;
            }

            runresult = runner::run(file_name, cpu_limit, mem_limit);
            if(runresult < 0)
            {
                statuscode = -2;
            }

            else if(runresult > 0)
            {
                statuscode = runresult;
            }
            else
            {
                statuscode = 0;
            }

        End:
            out_value["status"] = statuscode;
            out_value["reason"] = codetodesc(statuscode, file_name);
            if(statuscode == 0)
            {
                std::string _stdout;
                file_util::readfile(name_util::std_out(file_name), &_stdout, true);
                out_value["stdout"] = _stdout;
                std::string _stderr;
                file_util::readfile(name_util::std_err(file_name), &_stderr, true);
                out_value["stderr"] = _stderr;
            }

            Json::StyledWriter writer;
            *out_json = writer.write(out_value);

            //removetempfile(file_name);
        }
    };
}
