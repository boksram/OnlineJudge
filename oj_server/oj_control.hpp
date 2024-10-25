#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <mutex>
#include <cassert>
#include <fstream>
#include <json/json.h>
#include "../comm/httplib.h"
#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include "oj_model.hpp"
#include "oj_view.hpp"

namespace Control
{
    using namespace Log;
    using namespace Util;
    using namespace Model;
    using namespace View;
    using namespace httplib;
    
    class machine
    {
        public:
            machine()
                :_ip(""),_port(0),_load(0),_mtx(nullptr)
            {}
            ~machine(){}

            void load_increase()
            {
                if(_mtx) _mtx->lock();
                ++_load;
                if(_mtx) _mtx->unlock();
            }

            void load_decrease()
            {
                if(_mtx) _mtx->lock();
                --_load;
                if(_mtx) _mtx->unlock();
            }

            uint64_t load()
            {
                uint64_t tmp_load = 0;
                if(_mtx) _mtx->lock();
                tmp_load = _load;
                if(_mtx) _mtx->unlock();
                return tmp_load;
            }

            void reset_load()
            {
                if(_mtx) _mtx->lock();
                _load=0;
                if(_mtx) _mtx->unlock();
            }

            std::string _ip;
            int _port;
            uint64_t _load;
            std::mutex *_mtx;
    };

    const std::string service_machine = "./conf/service_machine.conf";
    class loadbalance
    {
        public:
            bool load_conf(const std::string &machine_conf)
            {
                std::ifstream in(machine_conf);
                if(!in.is_open())
                {
                    LOG(FATAL)<<"Failed to load machine config: "<<machine_conf<<"\n";
                    return false;
                }

                std::string line;
                while(std::getline(in,line))
                {
                    std::vector<std::string> tokens;
                    string_util::splitstring(line,&tokens,":");
                    if(tokens.size()!=2)
                    {
                        LOG(WARNING)<<"Failed to split: "<<line<<"\n";
                        continue;
                    }

                    machine m;
                    m._ip = tokens[0];
                    m._port = atoi(tokens[1].c_str());
                    m._load = 0;
                    m._mtx = new std::mutex();
                    _online.push_back(_machines.size());
                    _machines.push_back(m);
                }
                in.close();
                return true;
            }

            loadbalance()
            {
                assert(load_conf(service_machine));
                LOG(INFO)<<"Load machine config: "<<service_machine<<" success"<<"\n";
            }
            ~loadbalance(){}

            bool smart_choice(int *id, machine **m)
            {
                *id = _online[0];
                *m = &_machines[_online[0]];
                _mtx.lock();
                int online_num = _online.size();
                if(online_num==0)
                {
                    _mtx.unlock();
                    LOG(FATAL)<<"All backend compilation hosts are offline"<<"\n";
                    return false;
                }

                uint64_t min_load = _machines[_online[0]].load();
                for(int i=0;i<online_num;i++)
                {
                    uint64_t load = _machines[_online[i]].load();
                    if(min_load>load)
                    {
                        min_load = load;
                        *id=_online[i];
                        *m = &_machines[_online[i]];
                    }
                }
                _mtx.unlock();
                return true;
            }

            void offline_machine(int which)
            {
                _mtx.lock();
                for(auto iter = _online.begin(); iter!=_online.end();iter++)
                {
                    if(*iter==which)
                    {
                        _machines[which].reset_load();
                        _online.erase(iter);
                        _offline.push_back(which);
                        break;
                    }
                }
                _mtx.unlock();
            }

            void online_machines()
            {
                _mtx.lock();
                _online.insert(_online.end(), _offline.begin(), _offline.end());
                _offline.erase(_offline.begin(), _offline.end());
                _mtx.unlock();

                LOG(INFO)<<"All machines are back online"<<"\n";
            }

            void show_machines()
            {
                _mtx.lock();
                std::cout<<"Current list of online hosts";
                for(auto &id : _online)
                {
                    std::cout<<id<<" ";
                }
                std::cout<<std::endl;

                std::cout<<"Current list of off hosts";
                for(auto &id : _offline)
                {
                    std::cout<<id<<" ";
                }
                std::cout<<std::endl;
                _mtx.unlock();
            }

        private:
            std::vector<machine> _machines;
            std::vector<int> _online;
            std::vector<int> _offline;
            std::mutex _mtx;
    };

    class control
    {
        public:
            control()
            {}

            ~control()
            {}

            void recovery_machines()
            {
                _loadbalance.online_machines();
            }

            bool all_questions(std::string *html)
            {
                std::vector<question> all;
                if(_model.get_all_questions(&all))
                {
                    std::sort(all.begin(),all.end(), [](const question &q1, const question &q2){
                        return atoi(q1.number.c_str()) < atoi(q2.number.c_str());
                    });
                    _view.all_expand_html(all, html);
                }
                else
                {
                    *html = "Failed to get question list";
                    return false;
                }

                return true;
            }

            bool one_question(const std::string &number,std::string *html)
            {
                question q;
                if(_model.get_one_question(number,&q))
                {
                    _view.one_expand_html(q,html);
                }
                else
                {
                        std::cout<<"Post success"<<"\n";
                    *html = "question:" + number + "not exist";
                    return false;
                }

                return true;
            }

            void judge(const std::string &number, const std::string in_json, std::string *out_json)
            {
                question q;
                _model.get_one_question(number,&q);

                Json::Reader reader;
                Json::Value in_value;
                reader.parse(in_json,in_value);
                std::string code = in_value["code"].asString();

                Json::Value compile_value;
                compile_value["input"] = in_value["input"].asString();
                compile_value["code"] = code + "\n" + q.tail;
                compile_value["cpu_limit"] = q.cpu_limit;
                compile_value["mem_limit"] = q.mem_limit;

                Json::FastWriter writer;
                std::string compile_string = writer.write(compile_value);

                while(true)
                {
                    int id = 0;
                    machine *m = nullptr;
                    if(!_loadbalance.smart_choice(&id,&m))
                    {
                        break;
                    }

                    Client cli(m->_ip,m->_port);
                    m->load_increase();
                    LOG(INFO)<<"select success, host id: "<<id<<"  details: "<<m->_ip<<":"<<m->_port<<" Current host load: "<<m->load()<<"\n";
                    if(auto ret = cli.Post("/compile_run", compile_string, "application/json;charset=utf-8"))
                    {
                        if(ret->status == 200)
                        {
                            *out_json = ret->body;
                            m->load_decrease();
                            LOG(INFO)<<"Request service of compile_and_run success"<<"\n";
                            break;
                        }
                        m->load_decrease();
                    }
                    else
                    {
                        LOG(ERROR)<<"The host currently being requested may already be offline, host id: "<<id<<"  details: "<<m->_ip<<":"<<m->_port<<"\n";
                        _loadbalance.offline_machine(id);
                        _loadbalance.show_machines();
                    }
                }
            }

        private:
            model _model;
            view _view;
            loadbalance _loadbalance;
    };
}
