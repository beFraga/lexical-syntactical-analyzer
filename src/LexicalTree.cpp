#include "LexicalTree.h"
#include <stack>
#include <algorithm>
#include <iostream>
#include <queue>
#include <cstdlib>

LexicalTree::LexicalTree(const std::string & re) : _re(re), _root(nullptr), _leaf_counter(1) {
	_re.erase(std::remove_if(_re.begin(), _re.end(), ::isspace), _re.end());
	std::string exp_re = expand_groups(_re);
	_re = "(" + exp_re + ")#";
	generate_tree();
}

LexicalTree::~LexicalTree() {
	destroy_tree(_root);
}

void LexicalTree::destroy_tree(Node * node) {
	if (!node) return;
	destroy_tree(node->l);
	destroy_tree(node->r);
	delete node;
}

std::string LexicalTree::expand_groups(const std::string & re) {
	std::string res = "";
	for (size_t i = 0; i < re.size(); i++) {
		if (re[i] == '[') {
			size_t j = i + 1;
			std::string group = "";
			while (j < re.size() && re[j] != ']') {
				group += re[j];
				j++;
			}
			std::string expanded = "(";
			for (size_t k = 0; k < group.size(); k++) {
				if (k + 2 < group.size() && group[k+1] == '-') {
					char start = group[k];
					char end = group[k+2];
					for (char c = start; c <= end; c++) {
						expanded += c;
						expanded += "|";
					}
					k += 2;
				} else {
					expanded += group[k];
					expanded += "|";
				}
			}
			if (expanded.back() == '|') expanded.pop_back();
			expanded += ")";

			res += expanded;
			i = j;
		} else {
			res += re[i];
		}
	}
	return res;
}

std::string LexicalTree::insert_explicit_concat(const std::string & re) {
	std::string res = "";
	for (size_t i = 0; i < re.size(); i++) {
		res += re[i];
		if (i + 1 >= re.size()) continue;

		char c1 = re[i];
		char c2 = re[i + 1];
		bool is_c1_val = (c1 != '(' && c1 != '|' && c1 != '.');
		bool is_c2_val = (c2 != ')' && c2 != '|' && c2 != '.' && c2 != '*' && c2 != '+' && c2 != '?');
		if (is_c1_val && is_c2_val) res += '.';
	}

	return res;
}

int LexicalTree::precedence(char c) {
	if (c == '*' || c == '+' || c == '?') return 3;
	if (c == '.') return 2;
	if (c == '|') return 1;
	return 0;
}

std::string LexicalTree::infix_to_postfix(const std::string & re) {
	std::string postfix = "";
	std::stack<char> st;

	for (auto c : re) {
		if (c == '*' || c == '+' || c == '?' || c == '.' || c == '|' || c == '(' || c == ')') {
			if (c == '(')
				st.push(c);
			else if (c == ')') {
				while (!st.empty() && st.top() != '(') {
					postfix += st.top();
					st.pop();
				}
				if (!st.empty()) st.pop(); // Only one (
			} else  {
				while (!st.empty() && precedence(st.top()) >= precedence(c)) {
					postfix += st.top();
					st.pop();
				}
				st.push(c);
			}
		} else  {
			postfix += c;
		}
	}

	while (!st.empty()) {
		postfix += st.top();
		st.pop();
	}
	return postfix;
}


void LexicalTree::generate_tree() {
	std::cout << "Starting formating.." << std::endl;
	std::string exp_re = insert_explicit_concat(_re);
	std::string postfix = infix_to_postfix(exp_re);

	std::stack<Node *> st;

	auto safe_pop = [&st]() -> Node * {
		if (st.empty()) {
			std::cerr << "Fatal error: Bad format." << std::endl;
			exit(1);
		}
		Node * n = st.top();
		st.pop();
		return n;
	};
	std::cout << "Generating tree.." << std::endl;

	for (auto c : postfix) {
		if (c == '*' || c == '+' || c == '?') {
			NodeT type = (c == '*') ? NodeT::STAR : (c == '+' ? NodeT::CROSS : NodeT::OPT);
			Node * n = new Node(type, std::string(1, c));
			n->l = safe_pop();
			st.push(n);
		} else if (c == '.' || c == '|') {
			NodeT type = (c == '.') ? NodeT::CAT : NodeT::OR;
			Node * n = new Node(type, std::string(1, c));
			n->r = safe_pop();
			n->l = safe_pop();
			st.push(n);
		} else {
			Node * n = new Node(NodeT::VALUE, std::string(1, c), _leaf_counter++);
			st.push(n);
		}
	}

	_root = st.top();
	std::cout << "Computing sets.." << std::endl;
	compute_sets(_root);
}

void LexicalTree::compute_sets(Node * node) {
	if (!node) return;

	compute_sets(node->l);
	compute_sets(node->r);
	
	if (node->type == NodeT::VALUE) {
		// We need to calculate epsilon?
		node->nullable = false;
		node->firstpos.insert(node->id);
		node->lastpos.insert(node->id);
	}
	else if (node->type == NodeT::OR) {
		node->nullable = node->l->nullable || node->r->nullable;

		node->firstpos.insert(node->l->firstpos.begin(), node->l->firstpos.end());
		node->firstpos.insert(node->r->firstpos.begin(), node->r->firstpos.end());

		node->lastpos.insert(node->l->lastpos.begin(), node->l->lastpos.end());
		node->lastpos.insert(node->r->lastpos.begin(), node->r->lastpos.end());
	}
	else if (node->type == NodeT::CAT) {
		node->nullable = node->l->nullable && node->r->nullable;

		node->firstpos.insert(node->l->firstpos.begin(), node->l->firstpos.end());
		if (node->l->nullable)
			node->firstpos.insert(node->r->firstpos.begin(), node->r->firstpos.end());

		node->lastpos.insert(node->r->lastpos.begin(), node->r->lastpos.end());
		if (node->r->nullable)
			node->lastpos.insert(node->l->lastpos.begin(), node->l->lastpos.end());
	}
	else if (node->type == NodeT::STAR || node->type == NodeT::OPT) {
		node->nullable = true;

		node->firstpos.insert(node->l->firstpos.begin(), node->l->firstpos.end());
		node->lastpos.insert(node->l->lastpos.begin(), node->l->lastpos.end());
	}
	else if (node->type == NodeT::CROSS) {
		node->nullable = node->l->nullable;

		node->firstpos.insert(node->l->firstpos.begin(), node->l->firstpos.end());
		node->lastpos.insert(node->l->lastpos.begin(), node->l->lastpos.end());

	}
}

void LexicalTree::compute_followpos(Node * node, std::map<int, std::set<int>> & followpos) {
	if (!node) return;

	compute_followpos(node->l, followpos);
	compute_followpos(node->r, followpos);

	if (node->type == NodeT::CAT)
		for (int i : node->l->lastpos)
			followpos[i].insert(node->r->firstpos.begin(), node->r->firstpos.end());
	else if (node->type == NodeT::STAR || node->type == NodeT::CROSS)
		for (int i : node->l->lastpos)
			followpos[i].insert(node->l->firstpos.begin(), node->l->firstpos.end());
}

void LexicalTree::map_leaf_symbols(Node * node, std::map<int, char> & leaf_symbols, int & accept_pos) {
	if (!node) return;
	if (node->type == NodeT::VALUE) {
		if (node->value == "#") accept_pos = node->id;
		else leaf_symbols[node->id] = node->value[0];
	}

	map_leaf_symbols(node->l, leaf_symbols, accept_pos);
	map_leaf_symbols(node->r, leaf_symbols, accept_pos);
}

Automaton LexicalTree::build_dfa(const std::string & token) {
	Automaton dfa;
	std::map<int, std::set<int>> followpos;
	std::map<int, char> leaf_symbols;
	int accept_pos = -1;

	compute_followpos(_root, followpos);
	map_leaf_symbols(_root, leaf_symbols, accept_pos);

	std::vector<std::set<int>> d_states;
	std::queue<int> unmarked_states;

	std::set<int> s0 = _root->firstpos;
	d_states.push_back(s0);
	unmarked_states.push(0);

	dfa.addState(0);
	dfa.setInitialState(0);

	if (s0.find(accept_pos) != s0.end()) dfa.addFinalState(0, token);

	while (!unmarked_states.empty()) {
		int T_id = unmarked_states.front();
		unmarked_states.pop();
		std::set<int> T = d_states[T_id];

		std::set<char> alph;
		for (int p : T)
			if (leaf_symbols.count(p))
				alph.insert(leaf_symbols[p]);

		for (char a : alph) {
			std::set<int> U;
			for (int p : T)
				if (leaf_symbols.count(p) && leaf_symbols[p] == a)
					U.insert(followpos[p].begin(), followpos[p].end());

			if (U.empty()) continue;
			int target_id = -1;
			for (size_t i = 0; i < d_states.size(); i++) {
				if (d_states[i] == U) {
					target_id = i;
					break;
				}
			}

			if (target_id == -1) {
				target_id = d_states.size();
				d_states.push_back(U);
				unmarked_states.push(target_id);

				dfa.addState(target_id);
				if (U.find(accept_pos) != U.end())
					dfa.addFinalState(target_id, token);
			}
			dfa.addTransition(T_id, a, target_id);
		}
	}
	return dfa;
}
