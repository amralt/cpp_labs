#include <catch2/catch_test_macros.hpp>
#include <ish_dungeon.hpp>

#include <vector>
#include <string>


TEST_CASE("создание статьи", "[article]") {
	lab2::ArticleRepository repo;
	int id = repo.create_article("title", "content", {"tag1", "tag2"});
	REQUIRE(id == 1);
	auto article = repo.get_article(id);
	REQUIRE(article.has_value());
	REQUIRE(article->get().title == "title");
	REQUIRE(article->get().content == "content");
	REQUIRE(article->get().tags.size() == 2);
	REQUIRE(article->get().tags.count("tag1") == 1);
	REQUIRE(article->get().tags.count("tag2") == 1);
}



// ============================================
// CREATE ARTICLE
// ============================================

TEST_CASE("Create article", "[article]") {
    lab2::ArticleRepository repo;

    int id1 = repo.create_article(
        "First article",
        "hello world",
        {"cpp", "stl"}
    );

    int id2 = repo.create_article(
        "Second article",
        "some content",
        {"math"}
    );

    REQUIRE(id1 == 1);
    REQUIRE(id2 == 2);

    auto article = repo.get_article(id1);

    REQUIRE(article.has_value());

    REQUIRE(article->get().id == id1);
    REQUIRE(article->get().title == "First article");
    REQUIRE(article->get().content == "hello world");

    REQUIRE(article->get().likes == 0);
    REQUIRE(article->get().tags.size() == 2);
    REQUIRE(article->get().tags.count("cpp") == 1);
    REQUIRE(article->get().tags.count("stl") == 1);

    REQUIRE_FALSE(article->get().created_at.empty());

    // Проверяем обновление word_index
    int total = 0;
    auto results = repo.search("hello", 1, 10, total);

    REQUIRE(total == 1);
}


// ============================================
// GET ARTICLE
// ============================================

TEST_CASE("Get article", "[article]") {
    lab2::ArticleRepository repo;

    int id = repo.create_article(
        "Title",
        "Content",
        {"tag"}
    );

    SECTION("Existing article") {
        auto article = repo.get_article(id);

        REQUIRE(article.has_value());
        REQUIRE(article->get().id == id);
        REQUIRE(article->get().title == "Title");
    }

    SECTION("Non-existing article") {
        auto article = repo.get_article(999);

        REQUIRE_FALSE(article.has_value());
    }
}


// ============================================
// LIKE
// ============================================

TEST_CASE("Like article", "[like]") {
    lab2::ArticleRepository repo;

    int id = repo.create_article(
        "Title",
        "Content",
        {}
    );

    SECTION("Like existing article") {
        REQUIRE(repo.like(id));

        auto article = repo.get_article(id);
        REQUIRE(article->get().likes == 1);

        REQUIRE(repo.like(id));
        REQUIRE(article->get().likes == 2);
    }

    SECTION("Like non-existing article") {
        REQUIRE_FALSE(repo.like(999));
    }
}


// ============================================
// LIST ARTICLES
// ============================================

TEST_CASE("List articles", "[list]") {
    lab2::ArticleRepository repo;

    int id1 = repo.create_article(
        "Gamma",
        "content 1",
        {"cpp"}
    );

    int id2 = repo.create_article(
        "Alpha",
        "content 2",
        {"math"}
    );

    int id3 = repo.create_article(
        "Beta",
        "content 3",
        {"cpp"}
    );

    int total = 0;

    SECTION("List all articles") {
        auto ids = repo.list_articles(
            1, 10, "", "title", total
        );

        REQUIRE(total == 3);
        REQUIRE(ids.size() == 3);

        // Сортировка по названию
        REQUIRE(ids[0] == id2); // Alpha
        REQUIRE(ids[1] == id3); // Beta
        REQUIRE(ids[2] == id1); // Gamma
    }

    SECTION("Filter by tag") {
        auto ids = repo.list_articles(
            1, 10, "cpp", "title", total
        );

        REQUIRE(total == 2);
        REQUIRE(ids.size() == 2);

        REQUIRE(ids[0] == id3);
        REQUIRE(ids[1] == id1);
    }

    SECTION("Pagination") {
        auto ids = repo.list_articles(
            2, 1, "", "title", total
        );

        REQUIRE(total == 3);
        REQUIRE(ids.size() == 1);

        REQUIRE(ids[0] == id3);
    }

    SECTION("Sort by likes") {
        repo.like(id1);
        repo.like(id2);
        repo.like(id2);

        auto ids = repo.list_articles(
            1, 10, "", "likes", total
        );

        REQUIRE(ids.size() == 3);

        REQUIRE(ids[0] == id2);
        REQUIRE(ids[1] == id1);
        REQUIRE(ids[2] == id3);
    }
}


// ============================================
// SEARCH
// ============================================

TEST_CASE("Search articles", "[search]") {
    lab2::ArticleRepository repo;

    int id1 = repo.create_article(
        "First",
        "linear algebra",
        {}
    );

    int id2 = repo.create_article(
        "Second",
        "linear analysis",
        {}
    );

    int id3 = repo.create_article(
        "Third",
        "probability theory",
        {}
    );

    int total = 0;

    SECTION("Search by words") {
        auto results = repo.search(
            "linear algebra",
            1, 10, total
        );

        // Статьи объединяются по наличию хотя бы одного слова
        REQUIRE(total == 2);
        REQUIRE(results.size() == 2);

        // Первая статья содержит оба слова,
        // поэтому должна иметь большую релевантность.
        REQUIRE(results[0].id == id1);
        REQUIRE(results[1].id == id2);
    }

    SECTION("Duplicate words") {
        auto results = repo.search(
            "linear linear algebra",
            1, 10, total
        );

        REQUIRE(total == 2);
        REQUIRE(results.size() == 2);

        REQUIRE(results[0].id == id1);
    }

    SECTION("Unknown words") {
        auto results = repo.search(
            "unknownword",
            1, 10, total
        );

        REQUIRE(total == 0);
        REQUIRE(results.empty());
    }

    SECTION("Search pagination") {
        auto results = repo.search(
            "linear algebra",
            2, 1, total
        );

        REQUIRE(total == 2);
        REQUIRE(results.size() == 1);

        REQUIRE(results[0].id == id2);
    }
}

// ============================================
// RELATED
// ============================================

TEST_CASE("Related articles", "[related]") {
    lab2::ArticleRepository repo;

    int id1 = repo.create_article(
        "Target",
        "content",
        {"cpp", "stl", "algo"}
    );

    int id2 = repo.create_article(
        "Very similar",
        "content",
        {"cpp", "stl"}
    );

    int id3 = repo.create_article(
        "Less similar",
        "content",
        {"cpp"}
    );

    int id4 = repo.create_article(
        "Not similar",
        "content",
        {"math"}
    );

    SECTION("Jaccard similarity and sorting") {
        auto results = repo.related(id1, 10);

        REQUIRE(results.size() == 3);

        // J({cpp, stl, algo}, {cpp, stl}) = 2/3
        REQUIRE(results[0].id == id2);
        REQUIRE(results[0].score == 2.0 / 3.0); // на веру, что 1/3 == 1/3

        // J({cpp, stl, algo}, {cpp}) = 1/3
        REQUIRE(results[1].id == id3);
        REQUIRE(results[1].score == 1.0 / 3.0);

        // Исходная статья не должна попасть в результаты
        for (const auto& result : results) {
            REQUIRE(result.id != id1);
        }
    }

    SECTION("Limit results") {
        auto results = repo.related(id1, 2);

        REQUIRE(results.size() == 2);
        REQUIRE(results[0].id == id2);
        REQUIRE(results[1].id == id3);
    }

    SECTION("Zero limit") {
        auto results = repo.related(id1, 0);

        REQUIRE(results.empty());
    }

    SECTION("Non-existing article") {
        auto results = repo.related(999, 10);

        REQUIRE(results.empty());
    }
}


// ============================================
// TAG CLOUD
// ============================================

/*
TEST_CASE("Tag cloud", "[tag_cloud]") {
    lab2::ArticleRepository repo;
	
    repo.create_article(
        "First",
        "content",
        {"cpp", "stl"}
    );
	
    repo.create_article(
        "Second",
        "content",
        {"cpp"}
    );
	
    repo.create_article(
        "Third",
        "content",
        {"cpp", "math"}
    );
	
    auto tags = repo.tag_cloud();
	
    REQUIRE(tags.size() == 3);
	
    // cpp встречается в трёх статьях
    REQUIRE(tags[0].tag == "cpp");
    REQUIRE(tags[0].count == 3);
	
    // Остальные два тега встречаются по одному разу
}
*/

