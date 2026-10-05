#pragma once

#include <vector>
#include <string_view>
#include <string>

namespace lab1 {

class RadixTree {
public:
	RadixTree();

	bool contains(const std::string& s);

	void insert(const std::string& s);

	void remove(const std::string& s);

	std::vector<std::string> get_by_prefix(const std::string& prefix);

	int size();

	bool empty();

	RadixTree(std::initializer_list<std::string> list); 

	// RadixTree(const RadixTree& other);

	~RadixTree();

	// friend std::ostream& operator<<(std::ostream& os, const RadixTree& tree);

	// RadixTree& operator=(const RadixTree& other);

	// bool operator==(const RadixTree& other) const;

private:
	struct Node;
	
	struct Edge {
		std::string label;
		Node* targetNode;	
	};

	struct Node {
		std::vector<Edge> edges;
		bool isEndOfWord = false;
	};

	struct SearchResult{
		Node* node;
		Node* parent;
		Edge* edge;
		Node* grandparent = nullptr; 
		int matchedChars = 0;     // Сколько символов строки s мы прошли
		int commonPrefixLen = 0;  // Сколько символов совпало с ребром
		bool isFound;
	};

	Node* root;


	void dfs(Node& node, const std::string& prefix, std::vector<std::string>& words);

	SearchResult search(const std::string& s);

	void destroyNode(Node* node);
};

}
