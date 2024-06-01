#pragma once
#include <iostream>
#include <string>
#include <ctemplate/template.h>
#include "oj_model.hpp"

namespace View
{
    using namespace Model;
    const std::string template_path = "./template_html/";
    class view
    {
        public:
            view(){}
            ~view(){}

            void all_expand_html(std::vector<question> &questions, std::string *html)
            {
                std::string src_html = template_path + "all_questions.html";
                ctemplate::TemplateDictionary root("all_questions");
                for(const auto &q : questions)
                {
                    ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("question_list");
                    sub->SetValue("number",q.number);
                    sub->SetValue("title",q.title);
                    sub->SetValue("star",q.star);
                }

                ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html,ctemplate::DO_NOT_STRIP);
                tpl->Expand(html,&root);
            }

            void one_expand_html(question &q, std::string *html)
            {
                std::string src_html = template_path + "one_question.html";
                ctemplate::TemplateDictionary root("one_question");
                root.SetValue("number",q.number);
                root.SetValue("title",q.title);
                root.SetValue("star",q.star);
                root.SetValue("desc",q.desc);
                root.SetValue("pre_code",q.header);

                ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html,ctemplate::DO_NOT_STRIP);
                tpl->Expand(html,&root);
            }
    };
}
