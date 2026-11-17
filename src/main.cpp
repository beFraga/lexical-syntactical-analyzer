#include "LexicalTree.h"
#include "InputReader.h"
#include "Minimizer.h"
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

	for (auto &l : test) {
		for (auto &a : auts)
			std::cout << l << ": " << a.eval(l) << std::endl;
		std::cout << std::endl;
	}

	std::cout << "Teste de automatos reduzidos:" << std::endl;
	std::vector<Automaton> minAuts;
	minAuts.reserve(auts.size());
	for (Automaton &a : auts)
		minAuts.push_back(Minimizer::getMinimized(a));

	for (size_t i = 0; i < auts.size(); i++) {
		Automaton &min = minAuts[i];
		Automaton &a = auts[i];
		std::cout << "qtd estados original: " << a.getStates().size() << endl;
		std::cout << "qtd estados minimizado: " << min.getStates().size() << endl << endl;
		std::cout << "Comparação de resultados : (linha: original | minimizado)" << endl;
		for (auto &l : test) {
			std::cout << l << ": " << a.eval(l) << " | " << min.eval(l) << std::endl;
		}
		std::cout << std::endl;
	}
	return 0;
}
