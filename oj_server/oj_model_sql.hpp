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
#include "include/mysql.h"

namespace Model
{
    using namespace Log;
    using namespace Util;

    struct question
    {
        std::string number;
        std::string title;
        std::string star;
        std::string desc;
        std::string header;
        std::string tail;
        int cpu_limit;
        int mem_limit;
    };

    const std::string oj_questions = "oj_questions";
    const std::string host = "127.0.0.1";
    const std::string user = "oj_client";
    const std::string passwd = "123456";
    const std::string db = "oj";
    const int port = 3306;
    class model
    {
        public:
            bool QueryMysql(const std::string &sql,std::vector<question> *out)
            {
                MYSQL *my = mysql_init(nullptr);
                if(mysql_real_connect(my,host.c_str(),user.c_str(),passwd.c_str(),db.c_str(),port,nullptr,0)==nullptr)
                {
                    LOG(FATAL)<<"failed to connect to mysql!\n";
                    return false;
                }

                mysql_set_charactor_set(my,"utf8");
                LOG(INFO)<<"sucess to connect to mysql!\n";

                if(mysql_query(my,sql.c_str())!=0)
                {
                    LOG(WARNING)<<sql<<" execute error!\n";
                    return false;
                }

                MYSQL_RES *res = mysql_store_result(my);
                
                int rows = mysql_num_rows(res);
                int cols = mysql_num_fields(res);

                for(int i=0;i<rows;i++)
                {
                    MYSQL_ROW row = mysql_fetch_row(res);
                    question q;
                    q.number = row[0];
                    q.title = row[1];
                    q.star = row[2];
                    q.desc = row[3];
                    q.header = row[4];
                    q.tail = row[5];
                    q.cpu_limit = atoi(row[6]);
                    q.mem_limit = atoi(row[7]);

                    out->push_back(q);
                }

                free(res);
                mysql_close(my);
                return true;
            }

            bool get_all_questions(std::vector<question> *out)
            {
                std::string sql = "select * from ";
                sql+=oj_questions;
                return QueryMysql(sql,out);
            }

            bool get_one_question(const std::string &number, question *q)
            {
                std::string sql = "select * from ";
                sql+=oj_questions;
                sql+=" where number=";
                sql+=number;
                std::vector<question> result;
                if(QueryMysql(sql,&result))
                {
                    if(result.size()==1)
                    {
                        *q = result[0];
                        return true;
                    }
                }
                return false;
            }
    };
}
