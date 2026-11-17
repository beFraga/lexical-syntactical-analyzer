#include "Minimizer.h"

#include <algorithm>
#include <numeric>
#include <queue>

std::map<int, int> getReachableStates(const Automaton &a) {

    std::map<int, int> component;

    int ini = a.getInitialState();
    component[ini] = 0;
    std::queue<int> queue;
    queue.push(ini);
    while (!queue.empty()) {
        int at = queue.front();
        queue.pop();
        std::map<char, std::set<int>> transitions = a.getTransitions(at);
        for (auto &[f, to] : transitions) {
            if (to.begin() != to.end() ) {
                 at = *to.begin();
                if (!component.count(at)) {
                    component[at] = 0;
                    queue.push(at);
                }
            }
        }
    }
    return component;
}

Automaton Minimizer::getMinimized(const Automaton& a) {
    std::map<int, int> components = getReachableStates(a); // =0 => dead
    components[-1] = 0;
    bool hasFinal = false;
    for (int s : a.getFinalStates()) {
        if (std::map<int, int>::iterator it; (it = components.find(s)) != components.end()) {
            it->second = 1;
            hasFinal = true;
        }
    }

    if (!hasFinal)
        return {};

    int maxComp = 1;
    bool changed;
    do {
        changed = false;
        for (char symbol : a.getAlphabet()) {
            std::vector<std::pair<int, int>> compChanges;  //{state, newComp}
            std::map<std::pair<int, int>, int> transitions; //{sourceComp, destComp}
            std::vector<int> isFirst(maxComp + 1, true);

            for (auto [state, srcComp] : components) {
                if (srcComp == 0) continue;

                int dest = a.getNextState(state, symbol);
                int destComp = components[dest];
                std::pair<int, int> trans = {srcComp, destComp};
                if (isFirst[srcComp]) {
                    transitions[trans] = srcComp;
                    isFirst[srcComp] = false;
                } else {
                    auto t = transitions.find(trans);
                    if (t == transitions.end()) {
                        transitions[trans] = ++maxComp;
                        compChanges.emplace_back(state, destComp);
                    } else if (t->second != srcComp) {
                        compChanges.emplace_back(state, destComp);
                    }
                }
            }

            for (auto [state, newComp] : compChanges)
                components[state] = newComp;


            changed = changed || !compChanges.empty();
        }

    } while (changed);

    std::map<std::pair<int, char>, int> compTransitions;
    std::vector<bool> done(maxComp+1, false);
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
        if (components.count(f)) {
            ret.addFinalState(f);
        }
    }

    return ret;

}
