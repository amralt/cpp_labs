#pragma once

#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "article.hpp"

namespace lab2 {

struct SearchHit {
    int id = 0;
    // релеватность
    int score = 0;
};

struct RelatedHit {
    int id = 0;
    // схожесть с заданной статьёй
    double score = 0.0;
};

struct TagCount {
    std::string name;
    // количество статей с заданным тегом
    int count = 0;
};

class ArticleRepository {
public:
    // возвращает срез id для страницы
    // page - номер страницы, начиная с единицы
    // per_page - количество новостей на одной странице
    // tag - тег, с которым выбираются статьи. Если пуст, то выбираются все
    // sort - тип сортировки. Может принимать значения "likes" (сортировка по популярности), 
    //     "title" (сортировка по заголовку), "new" (сортировка по дате)
    // total - общее количество статей с заданным тегом
    std::vector<int> list_articles(
        int page,
        int per_page,
        const std::string& tag,
        const std::string& sort,
        int& total
    ) const;

    // возвращает опциональную ссылку на статью с заданным идентификатором.
    // std::optional - класс, экземпляр которого может хранить объект заданного типа или 
    //     ничего не хранить
    // std::reference_wrapper - класс-обёртка над ссылками. В некоторые шаблоны стандарт
    //     запрещает передавать ссылки, поэтому здесь он необходим.
    std::optional<std::reference_wrapper<const Article>> get_article(int id) const;

    // возвращает id статьи, созданной по заданным аргументам
    // как здесь избежать копирования и почему это лучше, чем принимать ссылки?
    int create_article(
        std::string title,
        std::string content,
        std::set<std::string> tags
    );

    // Возвращает набор с идентификаторами и релевантностями статей, которые удовлетворяют
    // запросу. Набор должен быть отсортирован по релевантности
    // query - запрос
    // page - номер страницы
    // per_page - количество статей на одной странице
    // total - общее количество статей, удовлетворяющих запросу
    std::vector<SearchHit> search(
        const std::string& query,
        int page, int per_page,
        int& total
    ) const;

    // возвращает набор с идентификаторами и степенью схожести статей. Набор должен быть 
    // отсортирован по степени схожести
    // id - идентификатор статьи, с которой сравниваем теги остальных статей
    // limit - ограничение количество похожих статей
    std::vector<RelatedHit> related(int id, size_t limit) const;

    // инкременирует количество лайков у статьи с заданным id
    // если статья с таким id не существует, возвращается false
    bool like(int id);

    // возвращает все теги c количеством статей, отсортированные по популярности
    std::vector<TagCount> tag_cloud() const;

private:
    // возвращает текущее время в текстовом формате. 
    static std::string now_iso8601();
    // разделяет строку на слова. Реализуйте или удалите, если не нужно
    static std::vector<std::string> tokenize(const std::string& text);
    
    static std::vector<int> paginate(
        const std::vector<int>& ids, 
        int page, 
        int per_page
    );

private:
    // контейнер с статьями их идентификаторами
    std::map<int, Article> articles;

    // вспомогательные контейнеры, которые позволяют получить быстрый
    // доступ к горячим данным

    // соотносит тег -> множество id статей, которые его содержат
    // необходим для быстрого поиска по тегам
    std::map<std::string, std::set<int>> tag_index;
    // соотносит слово -> множество id статей, которые его содержат
    // необходим для быстрого поиска по словам
    std::map<std::string, std::set<int>> word_index;

    // может пригодиться для генерации уникальных идентификаторов
    int next_id = 1;
};

}

