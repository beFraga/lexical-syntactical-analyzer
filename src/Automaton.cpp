#include "Automaton.h"
#include <iomanip>
#include <queue>
using namespace std;

Automaton::Automaton() : _initialState(0) {}

int Automaton::getInitialState() const {
    return _initialState;
}

const set<int>& Automaton::getStates() const {
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

void Automaton::setInitialState(int state) {
    _initialState = state;
    addState(state);
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

set<int> Automaton::getTransitions(int from, char symbol) const {
    auto it = _transitions.find({from, symbol});
    if (it != _transitions.end()) {
        return it->second;
    }
    return {};
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

set<int> Automaton::epsilonClosure(const Automaton & a, const set<int> & states) {
	set<int> closure = states;
	queue<int> q;

	for (int s : states) q.push(s);

	while (!q.empty()) {
		auto t = q.front();
		q.pop();
		set<int> epsDest = a.getEpsilonTransitions(t);
		for (int dest : epsDest)
			if (closure.find(dest) == closure.end()) {
				closure.insert(dest);
				q.push(dest);
			}
	}
	return closure;
}

Automaton Automaton::determinize(const Automaton & nfa) {
	Automaton dfa;

	auto alph = nfa.getAlphabet();
	vector<set<int>> dStates;
	queue<int> unmarkedStates;
	
	set<int> startSet = epsilonClosure(nfa, {nfa.getInitialState()});
	dStates.push_back(startSet);
	unmarkedStates.push(0);
	dfa.addState(0);
	dfa.setInitialState(0);

	for (int s : startSet)
		if (nfa.isAccepting(s)) {
			dfa.addFinalState(0, nfa.getTokenPattern(s));
			break;
		}
	
	while (!unmarkedStates.empty()) {
		int t = unmarkedStates.front();
		unmarkedStates.pop();
		set<int> T = dStates[t];

		for (char a : alph) {
			set<int> moveRes;
			for (int s : T) {
				set<int> dests = nfa.getTransitions(s, a);
				moveRes.insert(dests.begin(), dests.end());
			}

			if (moveRes.empty()) continue;

			set<int> U = epsilonClosure(nfa, moveRes);

			int target_id = -1;
			for (size_t i = 0; i < dStates.size(); i++)
				if (dStates[i] == U) {
					target_id = static_cast<int>(i);
					break;
				}
			
			if (target_id == -1) {
				target_id = static_cast<int>(dStates.size());
				dStates.push_back(U);
				unmarkedStates.push(target_id);

				dfa.addState(target_id);

				for (int s : U)
					if (nfa.isAccepting(s)) {
						dfa.addFinalState(target_id, nfa.getTokenPattern(s));
						break;
					}
			}
		dfa.addTransition(t, a, target_id);
		}
	}
	return dfa;
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
