#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
using namespace std;

struct Production {
    string head;
    vector<string> body;
};

class Grammar {
private:
    static inline const string _EPSILON = "&";
    const string _stackBottom = "$";
    
    // Não terminais, Terminais, Simbolo Inicial, Produções
    set<string> _terminals;
    set<string> _nonTerminals;
    string _initialSymbol;
    vector<Production> _productions;
    
    // Firsts e Follows
    map<string, set<string>> _firsts;
    map<string, set<string>> _follows;

public:
    Grammar();

    // Montar a gramática
    void addProduction(const string & head, const vector<string> & body);
    void setInitialSymbol(const string & i);

    // Consultas
    bool isTerminal(const string & s);
    bool isNonTerminal(const string & s);
    const vector<Production> & getProductions();

    // Calcular Firsts e Follows
    void computeFirst();
    void computeFollow();

    // Getters
    set<string> getFirst(const string & A);
    set<string> getFirst(const vector<string> & symbols);
    set<string> getFollow(const string & A);

    void print();
};