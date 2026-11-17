#pragma once
#include <string>
#include <set>
#include <vector>
#include <Automaton.h>

enum class NodeT { CAT, OR, VALUE, STAR, OPT, CROSS };

class LexicalTree {
public:
	struct Node {
		int id;
		NodeT type;
		Node * l;
		Node * r;
		std::string value;

		bool nullable;
		std::set<int> firstpos;
		std::set<int> lastpos;

		Node(NodeT type, std::string val = "", int id = 0)
			: id(id), type(type), l(nullptr), r(nullptr), value(val), nullable(false) {};
	};
	
	LexicalTree(const std::string & re);
	~LexicalTree();

	void generate_tree();
	void compute_sets(Node * node);

	Automaton build_dfa(const std::string & token);

	Node * get_root() const { return _root; }
private:
	std::string _re;
	Node * _root;
	int _leaf_counter;

	void destroy_tree(Node * node);
	std::string insert_explicit_concat(const std::string & re);
	std::string infix_to_postfix(const std::string & re);
	std::string expand_groups(const std::string & re);
	int precedence(char c);

	void compute_followpos(Node * node, std::map<int, std::set<int>> & followpos);
	void map_leaf_symbols(Node * node, std::map<int, char> & leaf_symbols, int & accept_pos);
};
