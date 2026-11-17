#include "LexicalTree.h"
#include "InputReader.h"
#include <iostream>

#define INPUT_FILE "input.txt"
#define TESTCASE_FILE "testcase.txt"

int main() {
	auto exprs = InputReader::readInput(INPUT_FILE);
	auto test = InputReader::readTest(TESTCASE_FILE);

	std::vector<Automaton> auts;
	for (auto [id, exp] : exprs) {
		std::cout << "Tag: " << id << " | Regular expression: "<< exp << std::endl;
		LexicalTree lextree(exp);
		Automaton a = lextree.build_dfa(id);
		auts.push_back(a);
		std::cout << std::endl;
	}

	for (auto l : test) {
		for (auto a : auts)
			std::cout << l << ": " << a.eval(l) << std::endl;
		std::cout << std::endl;
	}
	return 0;
}
