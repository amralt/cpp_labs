#include "article_repository.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <format>
#include <iterator>
#include <optional>

#include <article.hpp>


namespace lab2 {


std::string ArticleRepository::now_iso8601() {

    return std::format("{:%FT%TZ}", std::chrono::system_clock::now());;
}

std::vector<std::string> ArticleRepository::tokenize(const std::string& text) {
    std::istringstream stream(text);
    std::vector<std::string> tokens;
    std::string word;
    while (stream >> word) {
        tokens.push_back(word);
    }
    return {tokens};
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
    next_id++;

    Article newArticle{
        next_id,
        title,
        content,
        tags,
        0,
        now_iso8601()
    };

    articles[next_id]=newArticle;

    std::for_each(
        tags.begin(), tags.end(),
        [this](const std::string& tag) {
            tag_index[tag].insert(next_id);
        }
    );

    auto words = tokenize(content);
    std::for_each(
        words.begin(), words.end(),
        [this](const std::string& word) {
            word_index[word].insert(next_id);
        }
    );
    
    return {next_id};
}

std::optional<std::reference_wrapper<const Article>> ArticleRepository::get_article(int id) const {
    if (articles.find(id) != articles.end()) {
        return std::optional<std::reference_wrapper<const Article>>(articles.at(id));
    }

    return std::nullopt;
}

bool ArticleRepository::like(int id) {
    if (articles.find(id) == articles.end()) {
        return false;
    }
    articles.at(id).likes++;

    return true;
}

std::vector<int> ArticleRepository::list_articles(
    int page,
    int per_page,
    const std::string& tag,
    const std::string& sort,
    int& total
) const {
    std::vector<int> ids;
    int count = 0;
    std::for_each(
        articles.cbegin(), articles.cend(),
        [&ids, &count](const auto& entry) {
            if (tag.empty() || entry.second.tags.find(tag) != entry.second.tags.end()) {
                ids.push_back(entry.first);
                count++;
            }
        }
    );
    total=count;

    if (sort == "likes") {
        std::sort(
            ids.begin(), ids.end(),
            [this](int a, int b) {
                return articles.at(a).likes > articles.at(b).likes;
            }
        );
    } else if (sort == "title") {
        std::sort(
            ids.begin(), ids.end(),
            [this](int a, int b) {
                return articles.at(a).title < articles.at(b).title;
            }
        );
    } else if (sort == "new") {
        std::sort(
            ids.begin(), ids.end(),
            [this](int a, int b) {
                return articles.at(a).created_at > articles.at(b).created_at;
            }
        );
    }
    
    return paginate(ids, page, per_page);
}

std::vector<SearchHit> ArticleRepository::search(
    const std::string& query,
    int page, int per_page,
    int& total
) const {
    std::vector<std::string> unique_words = tokenize(query);

    if (unique_words.empty())
        return {};

    std::vector<SearchHit> results;
    std::map<int, int> id_to_score;
    
    std::for_each(articles.cbegin(), articles.cend(),
        [&](std::pair<int, Article> pair) {
            id_to_score[pair.first] = 0;
        });

    // очки релевантности прибавляются при совпадении хотя бы 1 слова. Если совпадет больше слов, то ничего не изменится.
    std::for_each(articles.cbegin(), articles.cend(),
        [&](const std::pair<int, Article>& pair) {
            std::for_each(
                unique_words.cbegin(),unique_words.cend(),
                [&](const std::string w) {
                    if (pair.second.content.find(w) != std::string::npos) {
                        id_to_score[pair.first]++;
                    }
                });
        });

    std::for_each(id_to_score.cbegin(), id_to_score.cend(),
        [&](std::pair<int, int> pair) {
            if(pair.second!=0) {
                results.push_back({pair.first, pair.second});
            }
        });

    if (results.empty()) {
        return {};
    }


    std::sort(results.begin(), results.end(),
        [&](const SearchHit& sh1, const SearchHit& sh2) {
            return sh1.score > sh2.score;
        });

    return results;
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
        
        hits.emplace_back(entry_id, score); // откуда next_id взять?
    }

    const auto n = std::min(limit, hits.size()); // надо минимум, т.к. hits может быть меньше
    std::sort(
        hits.begin(),  hits.end(),
        [](const RelatedHit& a, const RelatedHit& b) {
            return a.score > b.score;
        }
    );
    hits.resize(n);

    return hits;
}

std::vector<TagCount> ArticleRepository::tag_cloud() const {
    throw std::runtime_error("unimplemented");
    // TODO: надо отдельно хранить статистику? В какой переменной?
    return {};
}

}
