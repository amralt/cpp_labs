#include "radix_tree.hpp"
#include <string>



using namespace lab1;

RadixTree::RadixTree() : root(nullptr) {}


RadixTree::RadixTree(std::initializer_list<std::string> list) : root(nullptr) {
    for (const std::string& s : list) {
        insert(s);
    }
}


RadixTree::SearchResult RadixTree::search(const std::string& s) {
    Node* curNode = root;
    Node* prev = nullptr;
    Node* prevPrev = nullptr;
    Edge* usedEdge = nullptr;
    int checkedChars = 0;
    
    SearchResult res;

    while (curNode !=NULL && !curNode->edges.empty() && checkedChars < s.length()) {
        Edge *nextEdge = NULL;
        bool found = false;
        for (Edge& e: curNode->edges) 
            if (s.compare(checkedChars, e.label.size(), e.label) == 0) {
                prevPrev =prev ;
                prev = curNode;
                curNode = e.targetNode;
                usedEdge = &e;
                checkedChars += e.label.size();
                found = true;
                break;
            }
        if (!found) break;
    }
    
    res.grandparent = prevPrev;
    res.parent = prev;
    res.node = curNode;
    res.matchedChars = checkedChars;
    res.edge = usedEdge;
    if (checkedChars == s.length() && curNode!=nullptr && curNode->isEndOfWord) {
        res.isFound =true;
        return res;
    }
    
    res.isFound=false;
    if (curNode != nullptr && checkedChars < s.length()) {
        for (Edge& e : curNode->edges) {
            // Ищем ребро, с которым есть общее начало
            int c = 0;
            while (checkedChars + c < s.length() && 
                   c < e.label.size() && 
                   s[checkedChars + c] == e.label[c]) {
                c++;
            }
            if (c > 0) {
                res.commonPrefixLen = c;
                res.edge = &e; // Сохраняем ребро, которое будем делить
                break;
            }
        }
    }
    

    return res;
}


bool RadixTree::contains(const std::string& s) {
    RadixTree::SearchResult res= search(s);

    return (res.isFound);
};


void RadixTree::insert(const std::string &s){
    if (s.length() ==0) return;
    if (root == nullptr) {
        root = new Node();
    }

    SearchResult sr = search(s);

    if (sr.isFound)
        return;
    // была вершина на котором закончилось слово, но она не была помечена как конец
    if (sr.node != nullptr && sr.matchedChars == s.length()) {
        sr.node -> isEndOfWord=true;
        return;
    }

    // мы в тупике
    if (sr.node == nullptr) {
        Node* newNode = new Node();
        newNode->isEndOfWord = true;
        std::string endPart = s.substr(sr.matchedChars);
        sr.parent->edges.push_back({endPart, newNode});
        return;
    }
    // split
    if (sr.commonPrefixLen > 0 && sr.edge != nullptr) {
        Edge* edgeOld = sr.edge;
        Node* midNode = new Node();
        std::string oldLabel = edgeOld->label;
        Node* oldTarget = edgeOld->targetNode;

        edgeOld->label = edgeOld->label.substr(0, sr.commonPrefixLen);
        edgeOld->targetNode = midNode;
        
        midNode->edges.push_back({oldLabel.substr(sr.commonPrefixLen), oldTarget});

        // случай когда слово входит в ребро
        if (s.substr(sr.matchedChars + sr.commonPrefixLen).empty()){
            midNode->isEndOfWord=true;
        } else {
            Node* newNode=new Node();
            newNode->isEndOfWord=true;
            midNode->edges.push_back({s.substr(sr.matchedChars + sr.commonPrefixLen), newNode});
        }
        
        return;
    }
    // вообще ничего не подходит
    if (sr.node != nullptr) {
        Node* newNode = new Node();
        newNode->isEndOfWord = true;
        sr.node->edges.push_back({s.substr(sr.matchedChars), newNode});
        return;
    }
    
}


void RadixTree::remove(const std::string& s) {
    SearchResult sr = search(s);
    if (!sr.isFound) return;

    if (!sr.node->edges.empty() && sr.node->isEndOfWord) {
        sr.node->isEndOfWord = false;
        return;
    }
    
    if (sr.node == root) {
        delete root;
        root = nullptr;
        return;
    }
    Node* parent = sr.parent;
    std::vector<Edge>* edges = &parent->edges;
    int positionToDelete=-1;
    for (int i = 0; i < edges->size(); i++) {
        if (&(*edges)[i] == sr.edge) {
            positionToDelete = i;
        }
    }
    delete sr.node;
    edges->erase(edges->begin() + positionToDelete);

    if (parent != root && !parent->isEndOfWord && parent->edges.size() == 1) {
        if (sr.grandparent == nullptr) return; 

        Edge* edgeToParent = nullptr;
        for (Edge& e : sr.grandparent->edges) { 
            if (e.targetNode == parent) {
                edgeToParent = &e;
                break;
            }
        }
        if (edgeToParent == nullptr) return;

        Edge& childEdge = parent->edges[0];

        // ура, склеивем!
        edgeToParent->label += childEdge.label;
        edgeToParent->targetNode = childEdge.targetNode;

        delete parent;
    }
}


void RadixTree::dfs(Node& node, const std::string& prefix, std::vector<std::string>& words) {
    if (node.isEndOfWord) {
        words.push_back(prefix);
    }
    for (Edge& e : node.edges) {
        dfs(*e.targetNode, prefix+e.label, words);
    }
}


std::vector<std::string> RadixTree::get_by_prefix(const std::string& prefix) {
    SearchResult sr = search(prefix);
    std::vector<std::string> result{};

    if (sr.node == nullptr || sr.node->edges.empty()) return result;

    if (sr.node != nullptr && sr.matchedChars == prefix.length() && sr.node->isEndOfWord) {
        result.push_back(prefix);
    }

    if (sr.node != nullptr && sr.matchedChars == static_cast<int>(prefix.length())) {
        dfs(*sr.node, prefix, result);
    }

    return result;
}


int RadixTree::size() {
    if (root == nullptr) return 0;

    std::vector<std::string> words{};
    dfs(*root, "", words);
    return words.size();
}


bool RadixTree::empty() {
    return root == nullptr;
}


void RadixTree::destroyNode(Node* node) {
    if (node == nullptr) return;
    for (Edge& e : node->edges) {
        destroyNode(e.targetNode);
    }
    delete node;
}

RadixTree::~RadixTree() {
    destroyNode(root);
    root = nullptr;
}

