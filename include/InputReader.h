#pragma once
#include <string>
#include <vector>
#include <tuple>
#include "Grammar.h"
using namespace std;

/* Como usar:
#include "InputReader.h"
...
    // Linhas cruas do arquivo (sem as linhas vazias)
    vector<string> lines = InputReader::readLines("arquivo.txt");

    // T1: definições regulares ("nome: ER") -> vetor de (nome, ER)
    auto defs = InputReader::readRegexDefinitions("input.txt");

    // T2: gramática ("<A> ::= corpo | corpo") -> vetor de produções
    auto prods = InputReader::readGrammar("grammar.txt");
...

--- readRegexDefinitions e readGrammar usam readLines por baixo.
--- Linhas fora do formato esperado são ignoradas com um aviso.
*/

class InputReader {
private:
    static tuple<string, string> splitOnce(const string & line, const string & separator);
public:
    static vector<string> readLines(const string & addressToRead);
    static vector<tuple<string, string>> readRegexDefinitions(const string & addressToRead);
    static vector<Production> readGrammar(const string & addressToRead);
    static vector<string> tokenizer(const string & body);
};
