#include <catch2/catch_test_macros.hpp>
#include <radix_tree.hpp>
#include <set>
#include <iostream>

using namespace lab1;


TEST_CASE("добавление, поиск, удаление слова") {
	RadixTree tree;
    tree.insert("папа");
    REQUIRE(tree.contains("папа") == true);

	tree.insert("папа");
    REQUIRE(tree.contains("папа") == true);
	
	tree.insert("пара");
	REQUIRE(tree.contains("пара") == true);

	tree.insert("папа");
    REQUIRE(tree.contains("па") == false);

	REQUIRE(tree.size() == 2);

	std::vector<std::string> words = tree.get_by_prefix("па");
	REQUIRE(std::set<std::string>{"пара", "папа"} == std::set<std::string>(words.begin(), words.end()));

	tree.remove("папа");
	REQUIRE(tree.contains("папа") == false);


}

TEST_CASE("проверка что получение строк по префиксуу корректно и проверка работы конструктора копирования") {
	RadixTree tree {"папа", "пара", "па"}; 
    // tree.insert("папа");
	// tree.insert("пара");
	// tree.insert("па");


	std::vector<std::string> words = tree.get_by_prefix("па");
	REQUIRE(std::set<std::string>{"пара", "папа", "па"} == std::set<std::string>(words.begin(), words.end()));
	REQUIRE(tree.size() == 3);

}