#include "Automaton.h"
#include <iomanip>
using namespace std;

Automaton::Automaton() : _initialState(0) {}

void Automaton::setInitialState(int state) {
    _initialState = state;
    addState(state);
}

int Automaton::getInitialState() const {
    return _initialState;
}

const set<int>& Automaton::getStates() const{
    return _states;
}

const set<int>& Automaton::getFinalStates() const {
    return _acceptingStates;
}

const set<char>& Automaton::getAlphabet() const {
    return _alphabet;
}

bool Automaton::isAccepting(int state) const {
	return _acceptingStates.find(state) != _acceptingStates.end();
}

void Automaton::addState(int state) {
    _states.insert(state);
}

void Automaton::addFinalState(int state, const string& patternName) {
    addState(state);
    _acceptingStates.insert(state);
    if (!patternName.empty()) {
        _tokenPatterns[state] = patternName;
    }
}

void Automaton::addTransition(int from, char symbol, int to) {
    addState(from);
    addState(to);
    _alphabet.insert(symbol);
    _transitions[{from, symbol}].insert(to);
}

void Automaton::addEpsilonTransition(int from, int to) {
    addState(from);
    addState(to);
    _epsilonTransitions[from].insert(to);
}

set<int> Automaton::getTransitions(int from, char symbol) {
    auto it = _transitions.find({from, symbol});
    if (it != _transitions.end()) {
        return it->second;
    }
    return {};
}

map<char, set<int>> Automaton::getTransitions(int from) const {
    map<char, set<int>> res;
    pair<int, char> begin = {from, 0};
    for (auto it = _transitions.lower_bound(begin); it->first.first == from; ++it) {
        res[it->first.second] = it->second;
    }
    return res;
}

set<int> Automaton::getEpsilonTransitions(int from) const {
    auto it = _epsilonTransitions.find(from);
    if (it != _epsilonTransitions.end()) {
        return it->second;
    }
    return {};
}

string Automaton::getTokenPattern(int state) const {
    auto it = _tokenPatterns.find(state);
    if (it != _tokenPatterns.end()) {
        return it->second;
    }
    return "";
}

int Automaton::getNextState(int from, char symbol) const {
	auto it = _transitions.find({from, symbol});
	if (it != _transitions.end() && !it->second.empty())
		return *(it->second.begin());
	return -1;
}

string Automaton::eval(const string & input) const {
	int cur = _initialState;
	for (char c : input) {
		int next = getNextState(cur, c);
		if (next == -1) return "REJECTED";
		cur = next;
	}

	if (isAccepting(cur)) {
		string pattern = getTokenPattern(cur);
		return pattern.empty() ? "ACCEPTED" : pattern;
	}

	return "REJECTED";
}

Automaton Automaton::uniteAutomata(const vector<Automaton> & automata) {
    // Inicialização, criando estado novo
    Automaton resultant;
    resultant.setInitialState(0);

    for (const Automaton& a : automata) {
        // Calcular offset para não repetir estados
        int offset = *resultant.getStates().rbegin() + 1;

        // Adicionar transição epsilon para o estado inicial de 'a'
        resultant.addEpsilonTransition(resultant.getInitialState(), a.getInitialState() + offset);
    
        // Para cada estado de 'a', adicionar o estado e as transições no autômato resultante
        for (int state : a.getStates()) {
            resultant.addState(state+offset);
            for (const auto& [symbol, destinations] : a.getTransitions(state)) {
                for (int d : destinations) {
                    resultant.addTransition(state+offset, symbol, d+offset);
                }
            }
            
            // Adicionar transições epsilon
            for (int d : a.getEpsilonTransitions(state)) {
                resultant.addEpsilonTransition(state+offset, d+offset);
            }
        }

        // Adicionar estados finais
        for (int state : a.getFinalStates()) {
            resultant.addFinalState(state+offset, a.getTokenPattern(state));
        }
    }

    return resultant;
}

void Automaton::printTable() {
    cout << "\n=== TABELA DE TRANSIÇÕES DO AUTÔMATO ===" << endl;
    cout << "Estado Inicial: q" << _initialState << endl;
    cout << "Estados de Aceitação: ";
    for (int f : _acceptingStates) {
        cout << "q" << f << " ";
    }
    cout << "\n\n";

    // Cabeçalho da tabela
    cout << setw(10) << "Estado" << setw(9);
    for (char c : _alphabet) {
        cout << c << setw(10);
    }
    cout << setw(12) << "&" << endl;
    cout << string(30 + _alphabet.size() * 10, '-') << endl;

    // Linhas dos estados
    for (int state : _states) {
        string label = "q" + to_string(state);
        if (state == _initialState) label = "->" + label;
        if (_acceptingStates.count(state)) label += "*";

        cout << setw(10) << label;

        for (char c : _alphabet) {
            set<int> dest = getTransitions(state, c);
            string destStr = "";
            for (int d : dest) destStr += "q" + to_string(d) + " ";
            cout << setw(10) << (destStr.empty() ? "- " : destStr);
        }

        set<int> epsDest = getEpsilonTransitions(state);
        string epsStr = "";
        for (int d : epsDest) epsStr += "q" + to_string(d) + " ";
        cout << setw(12) << (epsStr.empty() ? "- " : epsStr) << endl;
    }
    cout << endl;
}
