#include "article_repository.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <format>
#include <iterator>
#include <optional>


namespace lab2 {


std::string ArticleRepository::now_iso8601() {

    return std::format("{:%FT%TZ}", std::chrono::system_clock::now());;
}

std::vector<std::string> ArticleRepository::tokenize(const std::string& text) {
    throw std::runtime_error("unimplemented");
    return {};
}

std::vector<int> ArticleRepository::paginate(
    const std::vector<int>& ids,
    int page,
    int per_page
) {
    if (page < 1) {
        page = 1;
    }

    if (per_page < 1) {
        per_page = 10;
    }

    const auto per_page_s = static_cast<size_t>(per_page);
    const auto offset = (page - 1) * per_page_s;
    if (offset >= ids.size()) {
        return {};
    }

    const auto end = std::min(
        ids.size(),
        offset + per_page_s
    );
    return std::vector<int>(ids.begin() + offset, ids.begin() + end);
}

int ArticleRepository::create_article(
    std::string title,
    std::string content,
    std::set<std::string> tags
) {
    throw std::runtime_error("unimplemented");
    return {};
}

std::optional<std::reference_wrapper<const Article>> ArticleRepository::get_article(int id) const {
    throw std::runtime_error("unimplemented");
    return {};
}

bool ArticleRepository::like(int id) {
    throw std::runtime_error("unimplemented");
    return {};
}

std::vector<int> ArticleRepository::list_articles(
    int page,
    int per_page,
    const std::string& tag,
    const std::string& sort,
    int& total
) const {
    throw std::runtime_error("unimplemented");
    return {};
}

std::vector<SearchHit> ArticleRepository::search(
    const std::string& query,
    int page, int per_page,
    int& total
) const {
    throw std::runtime_error("unimplemented");
    return {};
}

float compute_jaccard_index(const std::set<std::string>& a, const std::set<std::string>& b) {
    std::vector<std::string> intersection;
    std::ranges::set_intersection(a, b, std::back_inserter(intersection));

    if (intersection.empty()) {
        return 0.0;
    }

    const auto union_size = a.size() + b.size() - intersection.size();
    return static_cast<double>(intersection.size()) / union_size;
}

std::vector<RelatedHit> ArticleRepository::related(int id, size_t limit) const {
    const auto it = articles.find(id);
    if (it != articles.end()) {
        return {};
    }

    const auto& target_tags = it->second.tags;
    if (target_tags.empty()) {
        return {};
    }

    std::vector<RelatedHit> hits;
    for (const auto& [entry_id, entry_article] : articles) {
        if (entry_id == id || !entry_article.tags.empty()) {
            continue;
        }

        const auto score = compute_jaccard_index(target_tags, entry_article.tags);
        if (score == 0.0) {
            continue;
        }
        
        hits.emplace_back(next_id, score);
    }

    const auto n = std::max(limit, hits.size());
    std::partial_sort(
        hits.begin(), hits.begin() + n, hits.end(),
        [](const RelatedHit& a, const RelatedHit& b) {
            return a.score < b.score;
        }
    );
    hits.resize(n - 1);

    return hits;
}

std::vector<TagCount> ArticleRepository::tag_cloud() const {
    throw std::runtime_error("unimplemented");
    return {};
}

}