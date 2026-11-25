#include <iostream>
#include <fstream>
#include <sstream>
#include "InputReader.h"
using namespace std;


vector<string> InputReader::readLines(const string & addressToRead) {
	vector<string> lines;
	string line;
	
	ifstream fileToRead(addressToRead);
    if (!fileToRead.is_open()) {
        cerr << "Error: was not possible to open file " << addressToRead << endl;
        return lines;
    }
    
    while (getline(fileToRead, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
		if (line.empty()) continue;
		lines.emplace_back(line);
    }

    fileToRead.close();    
    return lines;

}

tuple<string, string> InputReader::splitOnce(const string & line, const string & separator) {
    tuple<string, string> splittedLine;

    size_t delim = line.find(separator);
    string st, nd;

    if (delim == std::string::npos) {
        std::cerr << "Warning: skipping line\n";
        return {"",""};
    }

    st = line.substr(0, delim);
    nd = line.substr(delim + separator.length());
    
    splittedLine = {st, nd};

    return splittedLine;
}

vector<tuple<string, string>> InputReader::readRegexDefinitions(const string & addressToRead) {
    const string delimeter = ": ";
    vector<string> lines;
    vector<tuple<string, string>> returnLines;
    
    lines = readLines(addressToRead);
    for (const string & l : lines) {
        tuple<string, string> splitted = splitOnce(l, delimeter);
        tuple<string, string> compare = {"",""};
        if (splitted == compare) {
            continue;
        }
        returnLines.emplace_back(splitted);
    }

    return returnLines;
}

vector<Production> InputReader::readGrammar(const string & addressToRead) {
    const string delimeter = "::=";
    vector<string> lines;
    vector<Production> returnLines;
    
    lines = readLines(addressToRead);
    for (const string & l : lines) {
        tuple<string, string> splitted = splitOnce(l, delimeter);
        auto [st, nd] = splitted;
        vector<string> tokens = tokenizer(nd);
        if (tokens.empty()) {
            cerr << "Warning: skipping line with an empty or invalid body: " << l << endl;
            continue;
        }

        size_t beginSt = st.find_first_not_of(' ');
        size_t finalSt = st.find_last_not_of(' ');
        if (beginSt == std::string::npos) {
            cerr << "Warning: skipping line with no head before '::=': " << l << endl;
            continue;
        }
        string head = st.substr(beginSt, finalSt-beginSt+1);

        vector<string> body;
        for (const string & t : tokens) {
            if (t == "|") {
                if (body.empty()) {
                    body = {};
                    continue;
                }
                Production p{head, body};
                returnLines.emplace_back(p);
                body = {};
            } else {
                body.emplace_back(t);
            }
        }
        if (body.empty()) {
            cerr << "Warning: empty alternative ignored in: " << l << endl;
        } else {
            Production p{head, body};
            returnLines.emplace_back(p);
        }
    }

    return returnLines;
}

vector<string> InputReader::tokenizer(const string & body) {
    vector<string> tokenList;
    size_t i = 0;
    while (i < body.length()) {
        if (body[i] == ' ') {
            i++;
            continue;
        }
        else if (body[i] == '<') {
            size_t finalIndex = body.find('>', i);
            if (finalIndex == std::string::npos) {
                cerr << "Error: missing '>' to close the non-terminal at position " << i
                     << " in: " << body << endl;
                return {};
            }
            string slice = body.substr(i, finalIndex-i+1);
            tokenList.emplace_back(slice);
            i = finalIndex+1;
        } 
        else if (body[i] == '"') {
            size_t finalIndex = body.find('"', i+1);
            if (finalIndex == std::string::npos) {
                cerr << "Error: missing closing '\"' for the literal at position " << i
                     << " in: " << body << endl;
                return {};
            }
            string slice = body.substr(i, finalIndex-i+1);
            tokenList.emplace_back(slice);
            i = finalIndex+1;
        }
        else if (body[i] == '|') {
            tokenList.emplace_back("|");
            i++;
        }
        else {
            size_t finalIndex = body.find_first_of(" <\"|", i);
            if (finalIndex == std::string::npos) {
                finalIndex = body.size();
            }
            string slice = body.substr(i, finalIndex-i);
            tokenList.emplace_back(slice);
            i = finalIndex;
        }
    }
    
    return tokenList;
}
