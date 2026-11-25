#include "Grammar.h"

Grammar::Grammar(){}

void Grammar::addProduction(const string & head, const vector<string> & body) {
    _nonTerminals.emplace(head);
    for (const string & b : body) {
        if (b == _EPSILON) continue;
        if (b.size() > 2 && b.front() == '<' && b.back() == '>') {
            _nonTerminals.emplace(b);
        } else {
            _terminals.emplace(b);
        }
    }
    Production p{head, body};
    _productions.emplace_back(p);
}

void Grammar::setInitialSymbol(const string & i) {
    _nonTerminals.emplace(i);
    _initialSymbol = i;
}

bool Grammar::isTerminal(const string & s) {
    if (_terminals.find(s) != _terminals.end()) {
        return true;
    }
    return false;
}

bool Grammar::isNonTerminal(const string & s) {
    if (_nonTerminals.find(s) != _nonTerminals.end()) {
        return true;
    }
    return false;
}

const vector<Production> & Grammar::getProductions() {
    return _productions;
}

void Grammar::computeFirst() {
    return;
}
void Grammar::computeFollow() {
    return;
}

set<string> Grammar::getFirst(const string & A) {
    return {};
}
set<string> Grammar::getFirst(const vector<string> & symbols) {
    return {};
}
set<string> Grammar::getFollow(const string & A) {
    return {};
}

void Grammar::print() {
    cout << "Simbolo inicial: " << _initialSymbol << endl << endl;

    cout << "Producoes:" << endl;
    for (size_t i = 0; i < _productions.size(); i++) {
        const Production & p = _productions[i];
        cout << "  (" << i << ") " << p.head << " ::=";
        for (const string & symbol : p.body)
            cout << " " << symbol;
        cout << endl;
    }

    cout << endl << "First:" << endl;
    for (const auto & [symbol, firstSet] : _firsts) {
        cout << "  First(" << symbol << ") = { ";
        for (const string & s : firstSet)
            cout << s << " ";
        cout << "}" << endl;
    }

    cout << endl << "Follow:" << endl;
    for (const auto & [symbol, followSet] : _follows) {
        cout << "  Follow(" << symbol << ") = { ";
        for (const string & s : followSet)
            cout << s << " ";
        cout << "}" << endl;
    }
}
