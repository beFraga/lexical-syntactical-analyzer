#include "Automaton.h"
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <queue>
#include <unordered_map>

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

map<int, int> Automaton::getReachableStates(const Automaton &a) {

	map<int, int> component;

	int ini = a.getInitialState();
	component[ini] = 0;
	queue<int> queue;
	queue.push(ini);
	while (!queue.empty()) {
		int at = queue.front();
		queue.pop();
		map<char, set<int>> transitions = a.getTransitions(at);
		for (auto &[f, to] : transitions) {
			if (to.begin() != to.end() ) {
				int nxt = *to.begin();
				if (!component.count(nxt)) {
					component[nxt] = 0;
					queue.push(nxt);
				}
			}
		}
	}
	return component;
}

Automaton Automaton::minimize(const Automaton& a) {
    map<int, int> components = getReachableStates(a); // =0 => dead
    components[-1] = 0;
    int maxComp = 0;
    bool hasFinal = false;
    unordered_map<string, int>  tokenComponent;
    for (int s : a.getFinalStates()) {
        if (map<int, int>::iterator it; (it = components.find(s)) != components.end()) {
            string token = a.getTokenPattern(s);
            int comp = tokenComponent[token];
            if (comp)
                it->second = comp;
            else
                it->second = tokenComponent[token] = ++maxComp;

            hasFinal = true;
        }
    }

    if (!hasFinal)
        return {};

    bool changed;
    do {
        changed = false;
        for (char symbol : a.getAlphabet()) {
            vector<pair<int, int>> compChanges;  //{state, newComp}
            map<pair<int, int>, int> transitions; //{sourceComp, destComp}
            vector<int> isFirst(maxComp + 1, true);

            for (auto [state, srcComp] : components) {
                int dest = a.getNextState(state, symbol);
                int destComp = components[dest];
                pair<int, int> trans = {srcComp, destComp};
                if (isFirst[srcComp]) {
                    transitions[trans] = srcComp;
                    isFirst[srcComp] = false;
                } else {
                    auto t = transitions.find(trans);
                    if (t == transitions.end()) {
                        transitions[trans] = ++maxComp;
                        compChanges.emplace_back(state, maxComp);
                    } else if (t->second != srcComp) {
                        compChanges.emplace_back(state, t->second);
                    }
                }
            }

            for (auto [state, newComp] : compChanges)
                components[state] = newComp;
            changed = changed || !compChanges.empty();
        }
    } while (changed);

    map<pair<int, char>, int> compTransitions;
    vector<bool> done(maxComp+1, false);
    done[0] = true;
    for (auto [state, comp] : components) {
        if (!done[comp]) {
            for (char symbol : a.getAlphabet()) {
                int destState = a.getNextState(state, symbol);
                int destComp = components[destState];
                compTransitions[{comp, symbol}] = destComp;
            }
            done[comp] = true;
        }
    }

    Automaton ret;
    for (int i = 1; i <= maxComp; i++)
        ret.addState(i);

    for (auto [k, to] : compTransitions) {
        if (to) {
            auto [from, symbol] = k;
            ret.addTransition(from, symbol, to);
        }
    }
    int iniS = a.getInitialState();
    int iniComp = components[iniS];
    ret.setInitialState(iniComp);

    for (int f : a.getFinalStates()) {
        auto it = components.find(f);
        if (it != components.end()) {
            string token = a.getTokenPattern(f);
            ret.addFinalState(it->second, token);
        }
    }
    return ret;
}

Automaton Automaton::unite(const vector<Automaton> & automata) {
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
