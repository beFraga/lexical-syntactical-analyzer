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

	auto a = Automaton::unite(auts);
	a.printTable();
	auto min_a = Automaton::minimize(a);
	min_a.printTable();

	for (auto &l : test)
		std::cout << l << ": " << a.eval(l) << std::endl;

	std::cout << "Teste de automatos reduzidos:" << std::endl;
	std::cout << "qtd estados original: " << a.getStates().size() << endl;
	std::cout << "qtd estados minimizado: " << min_a.getStates().size() << endl << endl;
	std::cout << "Comparação de resultados : (linha: original | minimizado)" << endl;
	for (auto &l : test)
		std::cout << l << ": " << a.eval(l) << " | " << min_a.eval(l) << std::endl;
	return 0;
}
