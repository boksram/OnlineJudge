#include <iostream>
#include <signal.h>
#include "../comm/httplib.h"
#include "oj_control.hpp"

using namespace httplib;
using namespace Control;

static control *ctrl_ptr = nullptr;

void recovery(int signo)
{
    ctrl_ptr->recovery_machines();
}

int main()
{
    signal(SIGQUIT, recovery);

    Server svr;
    control ctrl;
    ctrl_ptr = &ctrl;

    svr.Get("/all_questions",[&ctrl](const Request &req, Response &resp){
            std::string html;
            ctrl.all_questions(&html);
            resp.set_content(html,"text/html; charset=utf-8");
            });

    svr.Get(R"(/question/(\d+))",[&ctrl](const Request &req, Response &resp){
            std::string number = req.matches[1];
            std::string html;
            ctrl.one_question(number,&html);
            resp.set_content(html,"text/html; charset=utf-8");
            });

    svr.Post(R"(/judge/(\d+))",[&ctrl](const Request &req, Response &resp){
            std::string number = req.matches[1];
            std::string result_json;
            ctrl.judge(number,req.body,&result_json);
            resp.set_content(result_json, "application/json;charset=utf-8");
            });

    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0",8080);
    return 0;
}
