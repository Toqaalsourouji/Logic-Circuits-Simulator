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
