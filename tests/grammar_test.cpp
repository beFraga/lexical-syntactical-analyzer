#include "Grammar.h"

int main() {
    Grammar g;

    cout << "Teste" << endl;


    g.addProduction("<S>", {"<A>", "<S>", "<B>"});
    g.addProduction("<S>", {"&"});
    g.addProduction("<A>", {"a", "<A>"});
    g.addProduction("<A>", {"a"});
    g.addProduction("<B>", {"b", "<B>"});
    g.addProduction("<B>", {"&"});
    g.setInitialSymbol("<S>");

    cout << endl << "- is/isNonTerminal:" << endl;
    if (g.isTerminal("a")) {
        cout << "É terminal!" << endl;
    }
    if (g.isNonTerminal("<A>")) {
        cout << "É não terminal!" << endl;
    }

    cout << endl << "- getProductions:" << endl;
    for (Production p : g.getProductions()) {
        cout << p.head << "-> ";
        for (string b: p.body) {
            cout << b << " ";
        }
        cout << endl;
    }

    cout << endl << "Print Geral:" << endl;
    g.print();

    return 0;
}