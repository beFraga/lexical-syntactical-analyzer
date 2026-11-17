#pragma once
#include "Automaton.h"

class Minimizer {
public:
    static Automaton getMinimized(const Automaton &a);
};