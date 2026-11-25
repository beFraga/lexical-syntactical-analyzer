// Teste do tokenizador e do readGrammar do InputReader.
//
// Compilar e rodar (a partir da raiz do projeto, pois usa tests/grammar_bad.txt):
//   g++ -std=c++17 -Wall -Iinclude tests/reader_test.cpp src/InputReader.cpp src/Grammar.cpp -o reader_test
//   ./reader_test

#include <cstdio>
#include <fstream>
#include "InputReader.h"

// Imprime cada token entre colchetes, para ver espaços escondidos.
static void printTokens(const vector<string> & tokens) {
    for (const string & t : tokens)
        cout << "[" << t << "] ";
    cout << endl;
}

// Roda o tokenizador, mostra o resultado e compara com o esperado.
static void test(const string & entrada, const vector<string> & esperado) {
    vector<string> obtido = InputReader::tokenizer(entrada);

    cout << (obtido == esperado ? "[OK]   " : "[ERRO] ") << "entrada: " << entrada << endl;
    cout << "       obtido  : ";
    printTokens(obtido);
    if (obtido != esperado) {
        cout << "       esperado: ";
        printTokens(esperado);
    }
}

// Transforma uma produção em texto: "<A> -> a <A>". Facilita comparar.
static string format(const Production & p) {
    string s = p.head + " ->";
    for (const string & sym : p.body)
        s += " " + sym;
    return s;
}

// Lê um arquivo de gramática com readGrammar e compara com as produções esperadas.
static void testGrammar(const string & titulo, const string & arquivo, const vector<string> & esperado) {
    vector<string> obtido;
    for (const Production & p : InputReader::readGrammar(arquivo))
        obtido.push_back(format(p));

    cout << (obtido == esperado ? "[OK]   " : "[ERRO] ") << "readGrammar: " << titulo << endl;
    if (obtido != esperado) {
        cout << "       obtido:" << endl;
        for (const string & s : obtido) cout << "         [" << s << "]" << endl;
        cout << "       esperado:" << endl;
        for (const string & s : esperado) cout << "         [" << s << "]" << endl;
    }
}

int main() {
    // Passo 2: espaços e palavras soltas
    test("a b c",        {"a", "b", "c"});
    test("  a   b  ",    {"a", "b"});
    test("id",           {"id"});
    test("",             {});

    // Passo 3: não terminais <...>
    test("<A>",          {"<A>"});
    test("a <A>",        {"a", "<A>"});
    test("a<A>",         {"a", "<A>"});
    test("<A><S><B>",    {"<A>", "<S>", "<B>"});
    test("<A>b",         {"<A>", "b"});

    // Passo 4: terminais entre aspas (as aspas ficam no token)
    test("\"+\"",        {"\"+\""});
    test("<E> \"+\" <T>", {"<E>", "\"+\"", "<T>"});
    test("\"(\" <E> \")\"", {"\"(\"", "<E>", "\")\""});

    // Passo 5: separador de alternativas
    test("a | b",        {"a", "|", "b"});
    test("a|b",          {"a", "|", "b"});
    test("a <A> | &",    {"a", "<A>", "|", "&"});
    test("\"|\" a",      {"\"|\"", "a"});   // | entre aspas é terminal, não separador

    // Passo 6: inputs errados
    test("<A", {});
    test("\"abc", {});

    // readGrammar: linhas boas e ruins misturadas (avisos aparecem no cerr)
    // Linhas ignoradas: sem "::=", sem cabeça, e "<D ::= <D" (sem '>').
    const vector<string> esperado = {
        "<A> -> a <A>",
        "<A> -> a",
        "<B> -> b <B>",
        "<B> -> &",
        "<C> -> a",      // a | | b |  -> só as alternativas não vazias
        "<C> -> b",
        "<E> -> \"|\" x"
    };
    testGrammar("grammar_bad.txt", "tests/grammar_bad.txt", esperado);

    // O mesmo conteúdo com finais de linha do Windows (\r\n): resultado igual.
    const string crlf = "tests/grammar_bad_crlf.tmp";
    {
        ifstream in("tests/grammar_bad.txt");
        ofstream out(crlf, ios::binary);
        string linha;
        while (getline(in, linha))
            out << linha << "\r\n";
    }
    testGrammar("grammar_bad.txt com \\r\\n", crlf, esperado);
    remove(crlf.c_str());

    return 0;
}
