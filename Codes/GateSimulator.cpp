#include "GateSimulator.h"
#include "Gate.h"
#include "Event.h"
#include <fstream> 
#include <stack>   
#include <vector>  
#include <sstream> 
#include <iostream>
#include <algorithm> 
#include <set> 

using namespace std;

// Define a lambda function for comparing events by time, to be used with a priority queue.
// This ensures that events are processed in chronological order.
auto eventCompare = [](const Event& a, const Event& b) { return a.time < b.time; };
// Define a priority queue for storing and sorting events based on their time of occurrence.
priority_queue<Event, vector<Event>, decltype(eventCompare)> events(eventCompare);

// Define a map to associate gate names with their corresponding Gate objects.
unordered_map<string, Gate> gates;
// Define a map to associate signal names with a list of gates that use them as inputs.
unordered_map<string, vector<string>> signalToGates;
// Define a map to keep track of the current state (value) of each signal.
unordered_map<string, int> signalStates;

// Constructor for the CircuitSimulator class.
GateSimulator::GateSimulator(const string& libraryFile, const string& circuitFile, const string& stimuliFile)
{
    cout << "Parsing Library File...\n";
    parseLib(libraryFile); // Parse the library file to populate the libraryGates map.

    cout << "Parsing Circuit File...\n";
    parseCir(circuitFile); // Parse the circuit file to configure the gates in the simulation.

    cout << "Parsing Stimuli File...\n";
    parseStim(stimuliFile); // Parse the stimuli file to schedule initial events.

    // Initialize the outputs of gates based on their initial states before the simulation starts.
    initializeOutputs();
    // Print the configured gates and their connections for debugging purposes.
    printParsedCircuit();

    cout << "Initialization Complete.\n";
}
