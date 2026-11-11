#pragma once

#include <set>
#include <string>


struct Article {
    int id = 0;
    std::string title;
    std::string content;
    std::set<std::string> tags;
    int likes = 0;
    // дата в виде строки в формате, описанном в ISO-8601
    std::string created_at;
};
