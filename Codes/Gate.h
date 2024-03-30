#ifndef GATE_H
#define GATE_H

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

struct Gate {
    std::string type;
    std::vector<std::string> inputs;
    std::string output; // Output signal name
    std::string outputExpr; // Logical expression for output
    int delay;

    // Constructor is not necessary but helpful for initializing Gate objects cleanly
    Gate(std::string t = "", std::vector<std::string> i = {}, std::string o = "", std::string oe = "", int d = 0)
        : type(t), inputs(i), output(o), outputExpr(oe), delay(d) {}
};

extern std::unordered_map<std::string, Gate> gates;
extern std::unordered_map<std::string, std::vector<std::string>> signalToGates;


#endif // GATE_H
