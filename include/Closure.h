#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <set>
#include "Grammar.h"
using namespace std;

// Item LR(0): uma produção com um ponto marcando até onde já foi lida.
// Ex.: "<E> -> <E> • "+" <T>" tem prodId = índice dessa produção e dot = 1.
struct Item {
    int prodId;  // índice da produção em Grammar::getProductions()
    int dot;     // posição do ponto: 0 (antes do 1º símbolo) até body.size() (no fim)
    bool operator<(const Item & other) const {
        return tie(prodId, dot) < tie(other.prodId, other.dot);
    }
};
using State = set<Item>;

class Closure {
private:
    Grammar & _grammar;
public:
    Closure(Grammar & g);
    /*
    Fluxo do Closure
     Recebe "State items" e "Grammar"
        E' -> • E
     Devolve: State
        E' -> • E,
        E  -> • E + T,
        E  -> • T,
        T  -> • T * F,
        ...
    */
    State closure(const State & items);
};