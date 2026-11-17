#include "Minimizer.h"

#include <algorithm>
#include <numeric>
#include <queue>
#include <unordered_map>
using namespace std;

namespace {
    map<int, int> getReachableStates(const Automaton &a) {

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
}

Automaton Minimizer::getMinimized(const Automaton& a) {
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
    for (int i = 1; i <= maxComp; i++) {
        ret.addState(i);
    }

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
