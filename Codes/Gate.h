#pragma once
#ifndef GATE_H
#define GATE_H

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

using namespace std;

struct Gate {
    string type;
    vector<string> inputs;
    string output; // Output signal name
    string outputExpr; // Logical expression for output
    int delay;

    // Constructor is not necessary but helpful for initializing Gate objects cleanly
    Gate(string t = "", vector<string> i = {}, string o = "", string oe = "", int d = 0)
        : type(t), inputs(i), output(o), outputExpr(oe), delay(d) {}
};

extern unordered_map<string, Gate> gates;
extern unordered_map<string, vector<string>> signalToGates;


#endif // GATE_H
