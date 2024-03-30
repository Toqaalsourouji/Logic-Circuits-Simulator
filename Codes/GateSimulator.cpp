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


// Parse the circuit file to configure the simulation's gates and their connections.
void GateSimulator::parseCir(const string& filename)
{
    // Attempt to open the circuit file for reading.
    ifstream file(filename);
    // Check if the file opened successfully.
    if (!file.is_open())
    {
        // If not, report the error and return early.
        cerr << "Failed to open circuit file: " << filename << endl;
        return;
    }

    // Read the circuit file line by line.
    string line;
    while (getline(file, line))
    {
        // Skip empty lines.
        if (line.empty()) continue;

        // Parse each line to extract gate configurations.
        istringstream ss(line);
        string name, type, output;
        vector<string> componentInputs;

        // Extract the gate's name, type, output signal, and input signals.
        ss >> name >> type >> output;
        string input;
        while (ss >> input)
        {
            componentInputs.push_back(input);
        }

        // Check if the gate type exists in the library.
        if (libraryGates.find(type) != libraryGates.end())
        {
            // If so, adapt the gate's expression and configure the gate in the simulation.
            const Gate& circuitGate = libraryGates[type];
            string adaptedExpr = adaptExpression(circuitGate.outputExpr, componentInputs);
            gates[name] = Gate(type, componentInputs, output, adaptedExpr, circuitGate.delay);

            // Update the signalToGates and signalStates maps for the input and output signals.
            for (const auto& input : componentInputs)
            {
                signalToGates[input].push_back(name);
            }
            signalToGates[output].push_back(name);

            // Initialize the states of the input and output signals to 0.
            for (const auto& input : componentInputs)
            {
                signalStates.try_emplace(input, 0);
            }
            signalStates.try_emplace(output, 0);
        }
        else
        {
            // If the gate type is not found, report the error.
            cerr << "Gate type not found in library: " << type << endl;
        }
    }

    // Print the contents of the signalToGates map for debugging purposes.
    cout << "Debugging: Contents of signalToGates map after parsing circuit file:" << endl;
    for (const auto& entry : signalToGates)
    {
        cout << "Signal: " << entry.first << ", Associated Gates: ";
        for (const auto& gateName : entry.second)
        {
            cout << gateName << " ";
        }
        cout << endl;
    }
}

