#include "utils.hpp"
#define CPPHTTPLIB_THREAD_POOL_COUNT 1
#define CPPHTTPLIB_THREAD_POOL_MAX_COUNT 1
#include <httplib.h>
#include <nlohmann/json.hpp>


#include <functional>
#include <iostream>
#include <string>

#include "article_repository.hpp"
#include "utils.hpp"


using json = nlohmann::json;
using namespace lab2;

int main() {
    using namespace std::placeholders;

    ArticleRepository repo;
    httplib::Server svr;

    const char* env_www = std::getenv("ISH_DUNGEON_WWW");
	const char* www = env_www ? env_www : PROJECT_WWW_DIR;
	if (!svr.set_mount_point("/", www)) {
		std::cerr << "Cannot mount " << www << "\n";
		return 1;
	}

    svr.Get(
        "/api/articles",
        std::bind(handle_get_articles, std::cref(repo), _1, _2)
    );
    
    svr.Get(
        R"(/api/articles/(\d+))",
        std::bind(handle_get_article_by_id, std::cref(repo), _1, _2)
    );

    svr.Post(
        "/api/articles", 
        [&repo](const httplib::Request& request, httplib::Response& response) {
            handle_post_article(repo, request, response);
        }
    );

    svr.Get(
        "/api/search",
        std::bind(handle_get_search, std::cref(repo), _1, _2)
    );

    svr.Get(
        R"(/api/articles/(\d+)/related)",
        std::bind(handle_get_related, std::cref(repo), _1, _2)
    );

    svr.Post(
        R"(/api/articles/(\d+)/like)",
        [&repo](const httplib::Request& request, httplib::Response& response) {
            handle_post_like(repo, request, response);
        }
    );

    svr.Get(
        "/api/tags",
        std::bind(handle_get_tags, std::cref(repo), _1, _2)
    );

    svr.set_pre_routing_handler(handle_cors);

    svr.Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    std::cout << "Listening on http://0.0.0.0:8080\n";
    svr.listen("0.0.0.0", 8080);
}
