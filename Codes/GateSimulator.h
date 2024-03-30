#ifndef GATESIMULATOR_H
#define GATESIMULATOR_H

#include "Gate.h"
#include "Event.h"
#include <stack>
#include <string>
#include <unordered_map>
#include <queue>

using namespace std;

class GateSimulator {
private:
    unordered_map<string, Gate> libraryGates; // Map to store gate objects parsed from the library file
    unordered_map<string, Gate> circuitGates; // Map to store gate objects parsed from the circuit file
    unordered_map<string, int> signalStates; // Map to store signal states
    stack<Event> events; // Stack to store simulation events
    unordered_map<string, vector<string>> signalToGates;


public:
    GateSimulator(const string& libraryFile, const string& circuitFile, const string& stimuliFile);
    void startSimulation();
    void printGateInfo(const Gate& gate);
    void printInitialState();
    void printParsedCircuit();
    void initializeOutputs();
    void sortAndClean(const string& outputFile);

    
private:
    void parseLib(const string& filename);
    void parseCir(const string& filename);
    void parseStim(const string& filename);
    vector<string> infixToPostfix(const string& expression) const;
     int evaluateExpression(const string& expression, const vector<string>& inputs);
    void processEvents(const Event& event);
    void writeOutput(const string& signal, int value, int time); // Declaration of the new function
    string adaptExpression(const string& expression, const vector<string>& inputs);

    

};

#endif // GATESIMULATOR_H
