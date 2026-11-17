#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
using namespace std;

// Implementação de um Autômato Finito

class Automaton {
private:
    int _initialState;
    set<int> _states;
    set<int> _acceptingStates;
    set<char> _alphabet;

    // Transições normais (estado_origem -> símbolo) -> conjunto de estados destino
    map<pair<int, char>, set<int>> _transitions;

    // Transições vazias
    map<int, set<int>> _epsilonTransitions;

    // Mapeamento estado_de_aceitação -> token (ex: "id", "num")
    map<int, string> _tokenPatterns;

public:
    Automaton();

    // getters e setters
    void setInitialState(int state);
    int getInitialState();
    
    const set<int>& getStates();
    const set<int>& getFinalStates();
    const set<char>& getAlphabet();

	bool isAccepting(int state) const;

    // montar o autômato
    void addState(int state);
    void addFinalState(int state, const string& patternName = "");
    void addTransition(int from, char symbol, int to);
    void addEpsilonTransition(int from, int to);

    // consultar transições
    set<int> getTransitions(int from, char symbol);
    set<int> getEpsilonTransitions(int from);
    string getTokenPattern(int state) const;

	string eval(const std::string & input) const;
	int getNextState(int from, char symbol) const;

    // visualizar tabela de transições
    void printTable();
};
