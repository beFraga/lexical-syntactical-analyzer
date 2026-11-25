#include "LexicalTree.h"
#include "InputReader.h"
#include <iostream>

#define INPUT_FILE "input.txt"
#define TESTCASE_FILE "testcase.txt"

int main() {
	auto exprs = InputReader::readRegexDefinitions(INPUT_FILE);
	auto test = InputReader::readLines(TESTCASE_FILE);

	std::vector<Automaton> auts;
	for (auto [id, exp] : exprs) {
		std::cout << "Tag: " << id << " | Regular expression: "<< exp << std::endl;
		LexicalTree lextree(exp);
		Automaton a = lextree.build_dfa(id);
		auts.push_back(a);
		std::cout << std::endl;
	}

	auto a = Automaton::unite(auts);
	auto det_a = Automaton::determinize(a);
	auto min_a = Automaton::minimize(det_a);

	std::cout << "Teste de automatos:" << std::endl;
	std::cout << "Estados originais: " << det_a.getStates().size() << endl;
	std::cout << "Estados minimizados: " << min_a.getStates().size() << endl << endl;
	std::cout << "Comparação de resultados : (linha: determinizado | minimizado)" << endl;
	for (auto &l : test)
		std::cout << l << ": " << det_a.eval(l) << " | " << min_a.eval(l) << std::endl;
	return 0;
}
