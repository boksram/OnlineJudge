#include "compile_run.hpp"
#include "../comm/httplib.h"

using namespace httplib;
using namespace CompileAndRun;

void usage(std::string proc)
{
    std::cerr<<"usage: "<<"\n\t"<<proc<<" port"<<std::endl;
}

int main(int argc, char *argv[])
{
    if(argc!=2)
    {
        usage(argv[0]);
        return 1;
    }
    
    Server svr;
    svr.Post("/compile_run",[](const Request &req, Response &resp){
            std::string in_json = req.body;
            std::string out_json;
            if(!in_json.empty())
            {
                compileandrun::start(in_json,&out_json);
                resp.set_content(out_json,"application/json;charset=utf-8");
            }
        });

    svr.listen("0.0.0.0",atoi(argv[1]));

    return 0;
}
