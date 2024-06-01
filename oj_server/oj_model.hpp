#pragma once
#include <iostream>
#include <assert.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cstdlib>
#include "../comm/log.hpp"
#include "../comm/util.hpp"

namespace Model
{
    using namespace Log;
    using namespace Util;

    struct question
    {
        std::string number;
        std::string title;
        std::string star;
        int cpu_limit;
        int mem_limit;
        std::string desc;
        std::string header;
        std::string tail;
    };

    const std::string question_list = "./questions/questions.list";
    const std::string question_path = "./questions/";

    class model
    {
        public:
            model()
            {
                assert(loadquestionlist(question_list));
            }

            ~model()
            {}

            bool loadquestionlist(const std::string &question_list)
            {
                std::ifstream in(question_list);
                if(!in.is_open())
                {
                    LOG(FATAL)<<"Failed to load questions list, please check whether questions list is exist"<<"\n";
                    return false;
                }
                std::string line;
                while(getline(in,line))
                {
                    std::vector<std::string> tokens;
                    string_util::splitstring(line,&tokens," ");
                    if(tokens.size() != 5)
                    {
                        LOG(WARNING)<<"Failed to load some questions, please check format"<<"\n";
                        continue;
                    }
                    question q;
                    q.number = tokens[0];
                    q.title = tokens[1];
                    q.star = tokens[2];
                    q.cpu_limit = atoi(tokens[3].c_str());
                    q.mem_limit = atoi(tokens[4].c_str());

                    std::string path = question_path;
                    path += q.number;
                    path += "/";
                    file_util::readfile(path+"desc.txt", &(q.desc), true);
                    file_util::readfile(path+"header.cpp", &(q.header), true);
                    file_util::readfile(path+"tail.cpp", &(q.tail), true);

                    _questions.insert({q.number,q});
                }
                LOG(INFO)<<"Load questions list success"<<"\n";
                in.close();

                return true;
            }

            bool get_all_questions(std::vector<question> *out)
            {
                if(_questions.size()==0)
                {
                    LOG(ERROR)<<"Failed to get questions list"<<"\n";
                    return false;
                }
                for(const auto& q : _questions)
                {
                    out->push_back(q.second);
                }
                return true;
            }

            bool get_one_question(const std::string &number, question *q)
            {
                const auto &iter = _questions.find(number);
                if(iter == _questions.end())
                {
                    LOG(ERROR)<<"Failed to load the question, number:"<<number<<"\n";
                    return false;
                }
                *q = iter->second;

                return true;
            }

        private:
            std::unordered_map<std::string,question> _questions;
    };
}
