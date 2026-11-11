#pragma once

#define CPPHTTPLIB_THREAD_POOL_COUNT 1
#define CPPHTTPLIB_THREAD_POOL_MAX_COUNT 1
#include <httplib.h>
#include <nlohmann/json.hpp>


#include <string>

#include "article_repository.hpp"


namespace lab2 {

nlohmann::json article_to_json(const Article& article, bool include_content);

nlohmann::json search_hit_to_json(
    const ArticleRepository& repo,
    const SearchHit& hit
);

static nlohmann::json related_hit_to_json(
    const ArticleRepository& repo,
    const RelatedHit& hit
);

void send_json(httplib::Response& res, int status, const nlohmann::json& body);

void send_error(
    httplib::Response& res, int status,
    const std::string& message
);

int parse_int(const std::string& s, int default_value);

void handle_get_articles(
    const ArticleRepository& repo,
    const httplib::Request& request, 
    httplib::Response& response
);

void handle_get_article_by_id(
    const ArticleRepository& repo,
    const httplib::Request& request, 
    httplib::Response& response
);

void handle_post_article(
    ArticleRepository& repo,
    const httplib::Request& request, 
    httplib::Response& response
);

void handle_get_search(
    const ArticleRepository& repo,
    const httplib::Request& request,
    httplib::Response& response
);

void handle_get_related(
    const ArticleRepository& repo,
    const httplib::Request& request,
    httplib::Response& response
);

void handle_post_like(
    ArticleRepository& repo,
    const httplib::Request& request,
    httplib::Response& response
);

void handle_get_tags(
    const ArticleRepository& repo,
    const httplib::Request& request,
    httplib::Response& response
);

httplib::Server::HandlerResponse handle_cors(
    const httplib::Request& request,
    httplib::Response& response
);

}
