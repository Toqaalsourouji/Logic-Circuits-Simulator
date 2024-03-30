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

// Constructor for the GateSimulator class.
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


// Parse the library file to read and store gate definitions.
void GateSimulator::parseLib(const string& filename)
{
    // Attempt to open the library file for reading.
    ifstream file(filename);
    // Check if the file opened successfully.
    if (!file.is_open())
    {
        // If not, report the error and return early.
        cerr << "Failed to open library file: " << filename << endl;
        return;
    }

    // Read the library file line by line.
    string line;
    while (getline(file, line))
    {
        // Parse each line to extract gate information.
        istringstream ss(line);
        string componentName, token;
        int numOfInputs, delay;
        string outputExpr;

        // Extract the component name, number of inputs, logical expression, and delay from the line.
        getline(ss, componentName, ',');
        ss >> numOfInputs >> ws;  // Skip whitespace after numOfInputs
        ss.ignore();  // Skip the comma after numOfInputs

        getline(ss, outputExpr, ',');
        ss >> delay;

        // Create a Gate object and add it to the libraryGates map.
        libraryGates[componentName] = Gate(componentName, vector<string>(numOfInputs, ""), "", outputExpr, delay);

        // Print the parsed gate information for debugging.
        cout << "Parsed gate: " << componentName
            << " with expression: " << libraryGates[componentName].outputExpr
            << " and delay: " << delay << endl;
    }
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


// Print the configured gates and their connections for debugging purposes.
void GateSimulator::printParsedCircuit()
{
    cout << "Parsed Gates from Circuit File:" << endl;
    for (const auto& [name, gate] : gates) // Loop through each gate configured in the simulation.
    {
        // Print the gate's name, type, output signal, input signals, logical expression, and delay.
        cout << "Gate " << name << " with type " << gate.type << ", output " << gate.output
            << ", input(s): ";
        for (const auto& input : gate.inputs)
        {
            cout << input << " ";
        }
        cout << ", expression: " << gate.outputExpr << ", delay: " << gate.delay << endl;
    }
}


// Parse the stimuli file to schedule initial events for the simulation.
void GateSimulator::parseStim(const string& filename)
{
    // Attempt to open the stimuli file for reading.
    ifstream file(filename);
    // Check if the file opened successfully.
    if (!file.is_open())
    {
        // If not, report the error and return early.
        cerr << "Failed to open stimuli file: " << filename << endl;
        return;
    }

    // Read the stimuli file to extract and schedule events.
    string line;
    int time, value;
    string signal;

    // Extract the time, signal, and value for each event in the stimuli file.
    while (file >> time >> signal >> value)
    {
        // Print the parsed event information for debugging purposes.
        cout << "Parsed event: Time = " << time << ", Signal = " << signal << ", Value = " << value << endl;

        // Schedule the event by adding it to the events priority queue.
        events.push(Event(time, signal, value));

        // Associate the signal with the gates using it as an input and update the signal states.
        if (signalToGates.find(signal) != signalToGates.end())
        {
            for (const auto& gateName : signalToGates[signal])
            {
                Gate& gate = gates[gateName];
                // If the signal is an input to the gate, update the signal state.
                if (gate.inputs.end() != find(gate.inputs.begin(), gate.inputs.end(), signal))
                {
                    signalStates[signal] = value;
                }
            }
        }
    }
}


// Initialize gate outputs based on the initial states of their input signals.
void GateSimulator::initializeOutputs()
{
    cout << "Initializing gate outputs based on initial states...\n";
    for (auto& [gateName, gate] : gates)
    {
        // Evaluate the expression for each gate to determine its initial output value.
        int initOutputValue = evaluateExpression(gate.outputExpr, gate.inputs);
        // Update the state of the gate's output signal.
        signalStates[gate.output] = initOutputValue;
    }
}


// Start the simulation process by processing scheduled events.
void GateSimulator::startSimulation()
{
    cout << "Simulation starting. Total initial events: " << events.size() << endl;

    // Process each event in the priority queue until it's empty.
    while (!events.empty())
    {
        Event currentEvent = events.top(); // Get the next event to process.
        events.pop(); // Remove the event from the queue.
        processEvents(currentEvent); // Process the event.
    }

    // Indicate that the simulation is complete and all events have been processed.
    cout << "Simulation complete. No more events to process." << endl;
    sortAndClean("simulation.sim"); // Sort the output file and remove duplicate entries.
}


// Print the initial state of all signals and gates for debugging purposes.
void GateSimulator::printInitialState()
{
    cout << "Initial State:" << endl;
    // Loop through each signal and print its initial value.
    for (const auto& [signal, value] : signalStates)
    {
        cout << "Signal " << signal << " = " << value << endl;
    }
    // Print information about each gate configured in the simulation.
    cout << "Gates:" << endl;
    for (const auto& [name, gate] : gates)
    {
        cout << "Gate " << name << " with output " << gate.output << endl;
    }
}


// Additional helper function to print detailed information about a gate.
void GateSimulator::printGateInfo(const Gate& gate)
{
    // Print the gate's type, output signal, logical expression, and propagation delay.
    cout << "Gate Type: " << gate.type << ", Output: " << gate.output
        << ", Expression: " << gate.outputExpr << ", Delay: " << gate.delay << endl;
}


// Adapt a gate's expression from generic identifiers to actual signal names.
string GateSimulator::adaptExpression(const string& expression, const vector<string>& inputs)
{
    // Start with the original expression.
    string adaptedExpr = expression;
    // Iterate through each input signal.
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        // Construct the generic identifier (e.g., "i1" for the first input).
        string genericId = "i" + to_string(i + 1);

        // Replace all occurrences of this generic identifier with the actual input name.
        size_t pos = 0;
        while ((pos = adaptedExpr.find(genericId, pos)) != string::npos)
        {
            adaptedExpr.replace(pos, genericId.length(), inputs[i]);
            pos += inputs[i].length();
        }
    }
    // Return the expression with actual signal names.
    return adaptedExpr;
}


// Convert an infix logical expression to postfix notation for easier evaluation.
vector<string> GateSimulator::infixToPostfix(const string& expression) const
{
    stack<string> opStack; // Stack to hold operators during conversion.
    vector<string> output; // Vector to store the resulting postfix expression.
    // Define the precedence of logical operators.
    unordered_map<string, int> precedence{
        {"~", 3}, // Highest precedence for NOT operator.
        {"&", 2}, // AND operator precedence.
        {"|", 1}, // OR operator precedence.
        {"^", 1}  // XOR operator precedence, same as OR.
    };

    // Loop through each character in the expression to parse it.
    for (size_t i = 0; i < expression.size(); ++i)
    {
        // Handle operands (signal names or constants).
        if (isalnum(expression[i]))
        {
            string val;
            // Accumulate consecutive alphanumeric characters as a single operand.
            while (i < expression.size() && isalnum(expression[i]))
            {
                val += expression[i++];
            }
            --i; // Adjust the loop index after collecting the operand.
            output.push_back(val); // Add the operand to the output vector.
        }
        // Handle opening parentheses by pushing them onto the operator stack.
        else if (expression[i] == '(')
        {
            opStack.push("(");
        }
        // Handle closing parentheses by popping operators from the stack to the output until an opening parenthesis is found.
        else if (expression[i] == ')')
        {
            while (!opStack.empty() && opStack.top() != "(")
            {
                output.push_back(opStack.top());
                opStack.pop();
            }
            if (!opStack.empty()) opStack.pop(); // Pop the opening parenthesis.
        }
        // Handle operators.
        else
        {
            string op(1, expression[i]); // Treat the current character as an operator.
            // Pop operators with greater or equal precedence from the stack to the output.
            while (!opStack.empty() && precedence[opStack.top()] >= precedence[op])
            {
                output.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(op); // Push the current operator onto the stack.
        }
    }

    // After processing all characters, pop any remaining operators from the stack to the output.
    while (!opStack.empty())
    {
        output.push_back(opStack.top());
        opStack.pop();
    }
    return output; // Return the converted postfix expression.
}


// Evaluate a logical expression given in postfix notation and return the result.
int GateSimulator::evaluateExpression(const string& expression, const vector<string>& inputs)
{
    auto postfixExpr = infixToPostfix(expression); // Convert the infix expression to postfix notation.
    stack<int> evalStack; // Stack to hold intermediate evaluation results.

    // Debugging: Print the expression being evaluated along with the current values of inputs.
    cout << "Evaluating expression: " << expression << " with inputs: ";
    for (const auto& input : inputs)
    {
        cout << input << "(" << signalStates[input] << ") ";
    }
    cout << endl;

    // Loop through each token in the postfix expression for evaluation.
    for (const auto& token : postfixExpr)
    {
        // Handle logical operators by applying them to the operands on the evaluation stack.
        if (token == "~" || token == "&" || token == "|" || token == "^")
        {
            int result = 0; // Variable to store the result of the operation.
            // Special handling for the NOT operator, which is unary.
            if (token == "~")
            {
                int op1 = evalStack.top(); evalStack.pop();
                result = ~op1 & 1; // Perform NOT operation and ensure result is binary.
            }
            else
            {
                // For binary operators, pop two operands from the stack.
                int op2 = evalStack.top(); evalStack.pop();
                int op1 = evalStack.top(); evalStack.pop();

                // Perform the operation based on the operator token.
                if (token == "&") result = op1 & op2; // AND
                else if (token == "|") result = op1 | op2; // OR
                else if (token == "^") result = op1 ^ op2; // XOR
            }
            evalStack.push(result); // Push the result of the operation back onto the stack.
        }
        else
        {
            // For operands (signal names), push their current values onto the evaluation stack.
            evalStack.push(signalStates.at(token));
        }
    }

    // Return the result of the expression evaluation, which is the top value on the stack.
    return !evalStack.empty() ? evalStack.top() : 0;
}


// Write an event's details to the simulation output file.
void GateSimulator::writeOutput(const string& signal, int value, int time)
{
    // Open the simulation output file in append mode to add new events.
    ofstream simFile("simulation.sim", ios_base::app);
    if (simFile.is_open())
    {
        // Write the event details to the file: time, signal, and value.
        simFile << time << ", " << signal << ", " << value << endl;
    }
    else
    {
        // If the file cannot be opened, report the error.
        cerr << "Failed to open .sim file for writing." << endl;
    }
}


// Sort the simulation output file and remove duplicate entries.
void GateSimulator::sortAndClean(const string& outputFile)
{
    // Open the output file for reading to collect all lines.
    ifstream inputFile(outputFile);
    if (!inputFile.is_open())
    {
        // If the file cannot be opened, report the error and return early.
        cerr << "Failed to open output file: " << outputFile << endl;
        return;
    }

    // Read all lines from the file into a vector.
    vector<string> lines;
    string line;
    while (getline(inputFile, line))
    {
        lines.push_back(line);
    }

    // Close the input file after reading all lines.
    inputFile.close();

    // Sort the lines based on timestamps to ensure chronological order.
    sort(lines.begin(), lines.end(), [](const string& a, const string& b)
        {
            istringstream issA(a), issB(b);
    int timestampA, timestampB;
    char comma;
    issA >> timestampA >> comma; // Extract the timestamp from each line.
    issB >> timestampB >> comma;
    return timestampA < timestampB; // Compare timestamps for sorting.
        });

    // Remove duplicate lines from the vector.
    auto last = unique(lines.begin(), lines.end());
    lines.erase(last, lines.end());

    // Open the output file for writing, this time clearing its contents before writing.
    ofstream outputFileStream(outputFile, ofstream::out | ofstream::trunc);
    if (!outputFileStream.is_open())
    {
        // If the file cannot be opened for writing, report the error and return early.
        cerr << "Failed to open output file for writing: " << outputFile << endl;
        return;
    }

    // Write the sorted and deduplicated lines back to the output file.
    for (const auto& sortedLine : lines)
    {
        outputFileStream << sortedLine << '\n';
    }

    // Close the output file after writing.
    outputFileStream.close();

    // Inform the user that the output file has been sorted and cleaned.
    cout << "Output file sorted and repetitions removed: " << outputFile << endl;
}


// Process a single event, updating signal states and potentially triggering new events.
void GateSimulator::processEvents(const Event& currentEvent)
{
    // Log the details of the event being processed.
    cout << "Processing event for signal: " << currentEvent.signal
        << ", value: " << currentEvent.value
        << ", at time: " << currentEvent.time << endl;

    // Update the state of the signal involved in the event.
    signalStates[currentEvent.signal] = currentEvent.value;

    // Log the event details to the simulation output file.
    writeOutput(currentEvent.signal, currentEvent.value, currentEvent.time);

    // Check if the signal change affects any gates.
    if (signalToGates.find(currentEvent.signal) != signalToGates.end())
    {
        // Loop through each gate affected by the signal change.
        for (const auto& gateName : signalToGates[currentEvent.signal])
        {
            Gate& gate = gates[gateName]; // Reference to the affected gate.
            // Log the gate being checked and its configuration for debugging.
            cout << "Checking gate: " << gateName << " for signal: " << currentEvent.signal << endl;
            cout << "Gate " << gateName << " expression: " << gate.outputExpr << endl;
            cout << "Gate " << gateName << " inputs: ";
            for (const auto& input : gate.inputs)
            {
                cout << input << "(" << signalStates[input] << ") ";
            }
            cout << endl;

            // Debugging: Log the current signal states before evaluating the gate's output expression.
            cout << "Signal states before evaluation: ";
            for (const auto& [signal, value] : signalStates)
            {
                cout << signal << "(" << value << ") ";
            }
            cout << endl;

            // Evaluate the gate's output expression to determine if the gate's output changes.
            int oldOutputValue = signalStates[gate.output]; // The gate's output value before the event.
            int newOutputValue = evaluateExpression(gate.outputExpr, gate.inputs); // The potential new output value.

            // Log the result of the gate evaluation.
            cout << "Gate " << gateName
                << " Evaluated. Old Output: " << oldOutputValue
                << ", New Output: " << newOutputValue << endl;

            // If the gate's output has changed, schedule a new event to reflect this change.
            if (oldOutputValue != newOutputValue)
            {
                cout << "Output change detected. Scheduling new event for " << gate.output
                    << " at time " << (currentEvent.time + gate.delay)
                    << " with value " << newOutputValue << endl;

                // Update the gate's output signal state.
                signalStates[gate.output] = newOutputValue;
                // Add the new event to the events priority queue.
                events.push(Event(currentEvent.time + gate.delay, gate.output, newOutputValue));

                // Log the new event details to the simulation output file.
                writeOutput(gate.output, newOutputValue, currentEvent.time + gate.delay);
            }
        }
    }
    else
    {
        // If the signal change does not affect any gates, log this information for debugging.
        cout << "No gates associated with signal: " << currentEvent.signal << endl;
    }
}
