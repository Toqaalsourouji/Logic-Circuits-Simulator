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

// This comparator sorts events based on their 'time' attribute,
// ensuring the event with the latest time is at the top of the queue.
auto eventCompare = [](const Event& a, const Event& b) { return a.time < b.time; };

// Initialize a priority queue 'events' to store Event objects, sorted according to the 
// 'eventCompare' comparator.
priority_queue<Event, vector<Event>, decltype(eventCompare)> events(eventCompare);


unordered_map<string, Gate> gates;
unordered_map<string, vector<string>> signalToGates;
unordered_map<string, int> signalStates; // Assuming this is declared somewhere

// Constructor for the GateSimulator class: Initializes the simulator by parsing input files and setting up gate outputs.
GateSimulator::GateSimulator(const string& libraryFile, const string& circuitFile, const string& stimuliFile) {
    cout << "Parsing Library File...\n"; // Notify the user that the parsing of the library file is starting.
    parseLib(libraryFile);

    cout << "Parsing Circuit File...\n";// Notify the user that the parsing of the circuit file is starting.
    parseCir(circuitFile); // Parses the circuit file to build the internal representation of the circuit.

    cout << "Parsing Stimuli File...\n";// Notify the user that the parsing of the stimuli file is starting.
    parseStim(stimuliFile); // Parses the stimuli file to prepare the initial set of inputs for simulation.


    initializeGateOutputs(); // Initializes the outputs of the gates based on the initial simulation state.
    printParsedCircuitGates(); // Prints the gates of the parsed circuit for verification and debugging purposes.

    cout << "Initialization Complete.\n"; // Indicates to the user that the initialization process is complete.
}


// // Initializes the output states of all gates in the circuit based on their logical expressions and initial input states.
void GateSimulator::initializeGateOutputs() {
    cout << "Initializing gate outputs based on initial states...\n"; // Logs the start of gate output initialization.
    for (auto& [gateName, gate] : gates) { // Iterates over all gates in the circuit.
        int initOutputValue = evaluateExpression(gate.outputExpr, gate.inputs);
        signalStates[gate.output] = initOutputValue; //Updates the state of the gate's output signal in the signalStates map with the calculated initial value. 
    }
}

// Begins the simulation process, processes all scheduled events, and finalizes output.
void GateSimulator::startSimulation() {
    cout << "Simulation starting. Total initial events: " << events.size() << endl; // Logs the start of the simulation and the initial number of events.

    while (!events.empty()) { // Continues processing as long as there are events in the priority queue.
        Event currentEvent = events.top();
        events.pop();
        processEvents(currentEvent);
    }

    cout << "Simulation complete. No more events to process." << endl;
    sortAndClean("output.sim"); //Cleans and sorts the final output, then writes it to a file 
}

// Displays the initial state of all signals and the configuration of all gates before simulation starts
void GateSimulator::printInitialState() {
    cout << "Initial State:" << endl;
    for (const auto& [signal, value] : signalStates) {
        cout << "Signal " << signal << " = " << value << endl;
    }
    cout << "Gates:" << endl;
    for (const auto& [name, gate] : gates) {
        cout << "Gate " << name << " with output " << gate.output << endl; //Prints each gate's name and its initial output state
    }
}

// Prints detailed information about a specific gate, including its type, output signal, logical expression, and delay
void GateSimulator::printGateInfo(const Gate& gate) {
    cout << "Gate Type: " << gate.type << ", Output: " << gate.output
        << ", Expression: " << gate.outputExpr << ", Delay: " << gate.delay << endl; // Displays the gate's characteristics for debugging or verification purposes.
}

// Adapts a gate's generic expression to use the actual input signal names provided in the circuit description.
string GateSimulator::adaptExpression(const string& expression, const vector<string>& inputs) {
    string adaptedExpr = expression;
    for (size_t i = 0; i < inputs.size(); ++i) {
        // Construct the generic identifier (e.g., "i1" for the first input)
        string genericId = "i" + to_string(i + 1);

        // Replace all occurrences of this generic identifier with the actual input name
        size_t pos = 0;
        while ((pos = adaptedExpr.find(genericId, pos)) != string::npos) {
            adaptedExpr.replace(pos, genericId.length(), inputs[i]);
            pos += inputs[i].length();
        }
    }
    return adaptedExpr; // Returns the adapted expression with actual input names
}

// Parses the library file to build a map of gate types available for the simulation, including their characteristics
void GateSimulator::parseLib(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open library file: " << filename << endl;
        return;
    }

    string line; //Variable to hold each line read from the file
    while (getline(file, line)) {
        istringstream ss(line);
        string componentName, token;
        int numOfInputs, delay;
        string outputExpr;

        getline(ss, componentName, ',');
        ss >> numOfInputs >> ws;  // Skip whitespace after numOfInputs
        ss.ignore();  // Skip the comma after numOfInputs

        getline(ss, outputExpr, ',');
        ss >> delay;

        // Stores the gate definition in the libraryGates map with its characteristics
        libraryGates[componentName] = Gate(componentName, vector<string>(numOfInputs, ""), "", outputExpr, delay);


        //Logs the details of the parsed gate for verification
        cout << "Parsed gate: " << componentName
            << " with expression: " << libraryGates[componentName].outputExpr
            << " and delay: " << delay << endl;
    }
}

// Converts an infix logic expression into postfix notation for easier evaluation during simulation
vector<string> GateSimulator::infixToPostfix(const string& expression) const {
    stack<string> opStack;
    vector<string> output;
    unordered_map<string, int> precedence{
        {"~", 3}, // Unary NOT has the highest precedence
        {"&", 2}, // AND operator
        {"|", 1}, // OR operator
        {"^", 1}  // XOR operator, same precedence as OR
    };

    for (size_t i = 0; i < expression.size(); ++i) {
        if (isalnum(expression[i])) {
            string val;
            while (i < expression.size() && isalnum(expression[i])) {
                val += expression[i++];
            }
            --i; // Correcting the index since it's incremented in the loop above
            output.push_back(val); // Adds the operand to the output vector
        }
        else if (expression[i] == '(') { // If it's an opening parenthesis, push onto the stack
            opStack.push("(");
        }
        else if (expression[i] == ')') { // If it's a closing parenthesis, pop from stack to output until matching '('
            while (!opStack.empty() && opStack.top() != "(") {
                output.push_back(opStack.top());
                opStack.pop();
            }
            if (!opStack.empty()) opStack.pop(); //Removes the matching '(' from the stack
        }
        else {
            string op(1, expression[i]);
            while (!opStack.empty() && precedence[opStack.top()] >= precedence[op]) { // After processing the entire expression, pop any remaining operators to the output
                output.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(op);
        }
    }

    while (!opStack.empty()) { // After processing the entire expression, pop any remaining operators to the output
        output.push_back(opStack.top());
        opStack.pop();
    }

    return output;
}

// Parses the circuit file to build the simulation model by creating gate objects and setting up signal connections
void GateSimulator::parseCir(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open circuit file: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        istringstream ss(line);
        string name, type, output;
        vector<string> componentInputs;

        ss >> name >> type >> output; //Parses the gate's name, type, and output signal from the line
        string input;
        while (ss >> input) { // Reads all input signals for the gate
            componentInputs.push_back(input); //Adds each input signal to the componentInputs vector
        }
        // Checks if the gate type is defined in the library and constructs the gate object if it is
        if (libraryGates.find(type) != libraryGates.end()) {
            const Gate& libraryGate = libraryGates[type]; //Retrieves the gate definition from the library
            string adaptedExpr = adaptExpression(libraryGate.outputExpr, componentInputs);
            gates[name] = Gate(type, componentInputs, output, adaptedExpr, libraryGate.delay);
            // Initializes the states of the gate's inputs and output in the signalStates map, if they don't already exist
            for (const auto& input : componentInputs) {
                signalToGates[input].push_back(name);
            }
            signalToGates[output].push_back(name);

            for (const auto& input : componentInputs) {
                signalStates.try_emplace(input, 0);
            }
            signalStates.try_emplace(output, 0);
        }
        else {
            cerr << "Gate type not found in library: " << type << endl; //Logs an error if the gate type is not found in the library
        }
    }
    // Debugging output : prints the contents of the signalToGates map after parsing the circuit file
    cout << "Debugging: Contents of signalToGates map after parsing circuit file:" << endl;
    for (const auto& entry : signalToGates) {
        cout << "Signal: " << entry.first << ", Associated Gates: ";
        for (const auto& gateName : entry.second) {
            cout << gateName << " ";
        }
        cout << endl;
    }
}



// Displays a summary of all gates parsed from the circuit file, including their names, types, inputs, expressions, and delays
void GateSimulator::printParsedCircuitGates() {
    cout << "Parsed Gates from Circuit File:" << endl; // Logs a header to indicate the start of the gates summary
    for (const auto& [name, gate] : gates) { // Iterates over each gate in the 'gates' map
        cout << "Gate " << name << " with type " << gate.type << ", output " << gate.output
            << ", input(s): ";
        for (const auto& input : gate.inputs) {
            cout << input << " ";
        }
        cout << ", expression: " << gate.outputExpr << ", delay: " << gate.delay << endl;
    }
}

// Parses the stimuli file to schedule initial events and update signal states before starting the simulation
void GateSimulator::parseStim(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open stimuli file: " << filename << endl;
        return;
    }

    string line;
    int time, value;
    string signal;

    while (file >> time >> signal >> value) { //Reads each event (time, signal, value) from the file
        cout << "Parsed event: Time = " << time << ", Signal = " << signal << ", Value = " << value << endl;

        events.push(Event(time, signal, value));

        // Checks if the signal is used as an input to any gate
        if (signalToGates.find(signal) != signalToGates.end()) {
            for (const auto& gateName : signalToGates[signal]) {
                Gate& gate = gates[gateName];
                // Update the signal states if the signal is an input to a gate
                if (gate.inputs.end() != find(gate.inputs.begin(), gate.inputs.end(), signal)) {
                    signalStates[signal] = value; //Updates the state of the signal to the new value
                }
            }
        }
    }
}



// Evaluates a logical expression based on current signal states and returns the result
int GateSimulator::evaluateExpression(const string& expression, const vector<string>& inputs) {
    auto postfixExpr = infixToPostfix(expression); // Converts the infix expression to postfix for easier evaluation
    stack<int> evalStack; // Stack used for evaluation

    // Logs the expression being evaluated and the current states of its inputs
    cout << "Evaluating expression: " << expression << " with inputs: ";
    for (const auto& input : inputs) {
        cout << input << "(" << signalStates[input] << ") ";
    }
    cout << endl;

    for (const auto& token : postfixExpr) { // Iterates through each token in the postfix expression
        if (token == "~" || token == "&" || token == "|" || token == "^") { // If the token is an operator
            int result = 0; // Variable to store the result of the operation
            if (token == "~") {
                int op1 = evalStack.top(); evalStack.pop(); // Pops the top value for unary operation
                result = ~op1 & 1; // Assuming binary for simplicity
            }
            else {
                int op2 = evalStack.top(); evalStack.pop(); // Pops the top two values for binary operation
                int op1 = evalStack.top(); evalStack.pop();

                if (token == "&") result = op1 & op2;
                else if (token == "|") result = op1 | op2;
                else if (token == "^") result = op1 ^ op2;
            }
            // Pushes the result back onto the stack
            evalStack.push(result);
        }
        else {
            // Pushes the current state of the signal onto the stack for evaluation
            evalStack.push(signalStates.at(token));
        }
    }

    return !evalStack.empty() ? evalStack.top() : 0; // Returns the result of the expression evaluation (top of the stack)
}

// Writes a single simulation event to the output file, logging the time, signal, and value
void GateSimulator::writeOutput(const string& signal, int value, int time) {
    ofstream simFile("output.sim", ios_base::app);
    if (simFile.is_open()) {
        simFile << time << ", " << signal << ", " << value << endl; // Writes the time, signal name, and value to the file
    }
    else {
        cerr << "Failed to open .sim file for writing." << endl;
    }
}


// Sorts simulation output by time and removes duplicate entries, then writes the cleaned data back to the file
void GateSimulator::sortAndClean(const string& outputFile = "output.sim") {
    // Open the output file for reading
    ifstream inputFile(outputFile);
    if (!inputFile.is_open()) {
        cerr << "Failed to open output file: " << outputFile << endl;
        return;
    }

    // Read lines from the file into a vector
    vector<string> lines;
    string line; // Variable to hold each line read from the file
    while (getline(inputFile, line)) {
        lines.push_back(line);
    }

    // Close the input file
    inputFile.close();

    // Sort the lines based on timestamps
    sort(lines.begin(), lines.end(), [](const string& a, const string& b) {
        istringstream issA(a), issB(b);
    int timestampA, timestampB;
    char comma; // Variable to ignore commas during reading
    issA >> timestampA >> comma; // Extracts the timestamp from the first string
    issB >> timestampB >> comma; // Extracts the timestamp from the second string
    return timestampA < timestampB; // Compares timestamps for sorting
        });

    // Removes duplicate lines from the vector
    auto last = unique(lines.begin(), lines.end());
    lines.erase(last, lines.end());

    // Open the output file for writing (clearing its contents)
    ofstream outputFileStream(outputFile, ofstream::out | ofstream::trunc);
    if (!outputFileStream.is_open()) {
        cerr << "Failed to open output file for writing: " << outputFile << endl;
        return;
    }

    // Write the sorted and deduplicated lines back to the output file
    for (const auto& sortedLine : lines) {
        outputFileStream << sortedLine << '\n';
    }

    // Close the output file
    outputFileStream.close();

    cout << "Output file sorted and repetitions removed: " << outputFile << endl;
}

// Processes a single event from the simulation queue, updates signal states, and triggers any dependent gate evaluations
void GateSimulator::processEvents(const Event& currentEvent) {
    cout << "Processing event for signal: " << currentEvent.signal
        << ", value: " << currentEvent.value
        << ", at time: " << currentEvent.time << endl; // Logs details of the current event being processed

    // Update the state of the signal.
    signalStates[currentEvent.signal] = currentEvent.value;

    // Write the event to the output file
    writeOutput(currentEvent.signal, currentEvent.value, currentEvent.time);

    // Check if any gates are affected by this signal change.
    if (signalToGates.find(currentEvent.signal) != signalToGates.end()) {
        for (const auto& gateName : signalToGates[currentEvent.signal]) {
            Gate& gate = gates[gateName]; // Retrieves the gate object
            cout << "Checking gate: " << gateName << " for signal: " << currentEvent.signal << endl;

            // Debugging: Print gate expression and inputs
            cout << "Gate " << gateName << " expression: " << gate.outputExpr << endl;
            cout << "Gate " << gateName << " inputs: ";
            for (const auto& input : gate.inputs) {
                cout << input << "(" << signalStates[input] << ") ";
            }
            cout << endl;

            // Debugging: Print signal states before evaluating gate expression
            cout << "Signal states before evaluation: ";
            for (const auto& [signal, value] : signalStates) {
                cout << signal << "(" << value << ") ";
            }
            cout << endl;

            // Evaluates the gate's output based on the new signal states
            int oldOutputValue = signalStates[gate.output];
            int newOutputValue = evaluateExpression(gate.outputExpr, gate.inputs);

            // Logs the result of the gate evaluation and any output change
            cout << "Gate " << gateName
                << " Evaluated. Old Output: " << oldOutputValue
                << ", New Output: " << newOutputValue << endl;

            if (oldOutputValue != newOutputValue) { // // If the gate's output has changed.
                // Logs and schedules a new event for the updated gate output.
                cout << "Output change detected. Scheduling new event for " << gate.output
                    << " at time " << (currentEvent.time + gate.delay)
                    << " with value " << newOutputValue << endl;

                signalStates[gate.output] = newOutputValue; //  Updates the gate output signal state
                events.push(Event(currentEvent.time + gate.delay, gate.output, newOutputValue)); // Schedules the new event

                // Write the new event to the output file
                writeOutput(gate.output, newOutputValue, currentEvent.time + gate.delay);
            }
        }
    }
    else {
        cout << "No gates associated with signal: " << currentEvent.signal << endl;
    }
}
