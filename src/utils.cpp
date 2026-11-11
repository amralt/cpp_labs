#include "utils.hpp"

namespace lab2 {

using json = nlohmann::json;

json article_to_json(const Article& article, bool include_content) {
    json j;
    j["id"] = article.id;
    j["title"] = article.title;
    j["tags"] = article.tags;
    j["likes"] = article.likes;
    j["created_at"] = article.created_at;

    if (include_content) {
        j["content"] = article.content;
    }

    return j;
}

json search_hit_to_json(
    const ArticleRepository& repo,
    const SearchHit& hit
) {
    const auto article = repo.get_article(hit.id)->get();
    // article.content = "";
    auto j = article_to_json(article, false);
    j["score"] = hit.score;
    return j;
}

static json related_hit_to_json(
    const ArticleRepository& repo,
    const RelatedHit& hit
) {
    const auto article = repo.get_article(hit.id)->get();
    auto j = article_to_json(article, false);
    j["score"] = hit.score;
    return j;
}

void send_json(httplib::Response& res, int status, const json& body) {
    res.status = status;
    res.set_content(body.dump(), "application/json; charset=utf-8");
}

void send_error(
    httplib::Response& res, int status,
    const std::string& message
) {
    send_json(res, status, json{{"error", message}});
}

int parse_int(const std::string& s, int default_value) {
    try {
        return std::stoi(s);
    } catch (...) {
        return default_value;
    }
}

void handle_get_articles(
    const ArticleRepository& repo,
    const httplib::Request& request, 
    httplib::Response& response
) {
    const auto page = parse_int(request.get_param_value("page"), 1);
    const auto per_page = std::min(parse_int(request.get_param_value("per_page"), 10), 50);
    const std::string tag  = request.get_param_value("tag");
    const std::string sort = request.get_param_value("sort");

    int total = 0;
    const auto ids = repo.list_articles(page, per_page, tag, sort, total);

    json articles = json::array();
    for (int id : ids) {
        auto article = repo.get_article(id)->get();
        articles.push_back(article_to_json(article, false));
    }

    send_json(response, 200, json{
        {"page", page},
        {"per_page", per_page},
        {"total", total},
        {"articles", articles}
    });
}

void handle_get_article_by_id(
    const ArticleRepository& repo,
    const httplib::Request& request, 
    httplib::Response& response
) {
    const auto id = std::stoi(request.matches[1]);
    const auto article = repo.get_article(id);
    if (!article.has_value()) {
        send_error(response, 404, "article not found");
        return;
    }

    send_json(response, 200, article_to_json(article->get(), true));
}

void handle_post_article(
    ArticleRepository& repo,
    const httplib::Request& request, 
    httplib::Response& response
) {
    const auto content_type = request.get_header_value("Content-Type");
    if (content_type.find("application/json") == std::string::npos) {
        send_error(response, 415, "Content-Type must be application/json");
        return;
    }

    json body;
    try {
        body = json::parse(request.body);
    } catch (...) {
        send_error(response, 400, "invalid json");
        return;
    }

    if (!body.contains("title") || !body["title"].is_string() ||
        !body.contains("content") || !body["content"].is_string()) {
        send_error(response, 400, "fields 'title' and 'content' are required");
        return;
    }

    std::set<std::string> tags;
    if (body.contains("tags")) {
        if (!body["tags"].is_array()) {
            send_error(response, 400, "'tags' must be an array of strings");
            return;
        }
        for (const auto& t : body["tags"]) {
            if (!t.is_string()) {
                send_error(response, 400, "'tags' must be an array of strings");
                return;
            }
            tags.insert(t.get<std::string>());
        }
    }

    const auto id = repo.create_article(
        body["title"].get<std::string>(),
        body["content"].get<std::string>(),
        tags
    );

    const auto article = repo.get_article(id);
    send_json(response, 201, article_to_json(article->get(), true));
}

void handle_get_search(
    const ArticleRepository& repo,
    const httplib::Request& request,
    httplib::Response& response
) {
    const auto q = request.get_param_value("q");
    const auto page = parse_int(request.get_param_value("page"), 1);
    const auto per_page = std::min(parse_int(request.get_param_value("per_page"), 10), 50);

    int total = 0;
    const auto hits = repo.search(q, page, per_page, total);

    auto results = json::array();
    for (const auto& hit : hits) {
        results.push_back(search_hit_to_json(repo, hit));
    }

    send_json(response, 200, json{
        {"query", q},
        {"page", page},
        {"per_page", per_page},
        {"total", total},
        {"results", results}
    });
}

void handle_get_related(
    const ArticleRepository& repo,
    const httplib::Request& request,
    httplib::Response& response
) {
    const auto id = std::stoi(request.matches[1]);
    const auto limit = parse_int(request.get_param_value("limit"), 5);

    const auto article = repo.get_article(id);
    if (!article.has_value()) {
        send_error(response, 404, "article not found");
        return;
    }

    const auto related = repo.related(id, static_cast<size_t>(limit));
    auto arr = json::array();
    for (const auto& hit : related) {
        arr.push_back(related_hit_to_json(repo, hit));
    }

    send_json(response, 200, json{
        {"article_id", id},
        {"related", arr}
    });
}

void handle_post_like(
    ArticleRepository& repo,
    const httplib::Request& request,
    httplib::Response& response
) {
    const auto id = std::stoi(request.matches[1]);
    if (!repo.like(id)) {
        send_error(response, 404, "article not found");
        return;
    }
    const auto article = repo.get_article(id)->get();
    send_json(response, 200, json{
        {"id", article.id},
        {"likes", article.likes}
    });
}

void handle_get_tags(
    const ArticleRepository& repo,
    const httplib::Request& request,
    httplib::Response& response
) {
    const auto tags = repo.tag_cloud();

    auto arr = json::array();
    for (const auto& t : tags) {
        arr.push_back(json{{"name", t.name}, {"count", t.count}});
    }

    send_json(response, 200, json{{"tags", arr}});
}

httplib::Server::HandlerResponse handle_cors(
    const httplib::Request& request,
    httplib::Response& response
) {
    response.set_header("Access-Control-Allow-Origin", "*");
    response.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response.set_header("Access-Control-Allow-Headers", "Content-Type");
    return httplib::Server::HandlerResponse::Unhandled;
}

}
