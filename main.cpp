#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>

using namespace std;

// Define the Component class
class Component
{
public:
	string name;
	string type;
	string output;
	vector < string > inputs;
	int delay;
	string output_expression;		// Added field for output expression

	Component()
	{
	}

	Component(string name, string type, vector < string > inputs,
		string output, int delay) :name(name), type(type),
		inputs(inputs), output(output), delay(delay)
	{
	}


	int evaluateLogic(const unordered_map < string, int >& inputExpression,
		const unordered_map < string, int >& wires)
	{
		int result = (type == "AND2" || type == "NAND") ? 1 : 0;	// Assume true for AND/NAND until proven otherwise
		int inputVal = result;
		if (inputs.empty() || wires.empty())
		{
			cerr << "Error: Missing inputs or wires.\n";
			return -1;
		}
		for (const auto& input : inputs)
		{

			if (type == "AND2" || type == "NAND")
			{
				result &= inputVal;
			}
			else if (type == "OR2" || type == "NOR")
			{
				result |= inputVal;
			}
			else
				if (type == "XOR")
				{
					result ^= inputVal;
				}	// Special handling for NOT gates is outside this loop, as they only have one input
		}



		if (type == "NAND" || type == "NOR")
		{
			result = !result;	// Negate the result for NAND and NOR after processing all inputs
		}

		// For NOT gates, handle outside the loop since they should have exactly one input
		if (type == "NOT")
		{
			if (inputs.size() != 1)
			{
				cerr << "Error: NOT gate should have exactly one input\n";
				return -1;
			}
			try
			{
				const auto& inputWire = inputs.front();
				int inputVal =
					(inputExpression.find(inputWire) !=
						inputExpression.end()) ? inputExpression.at(inputWire) : wires.
					at(inputWire); return !inputVal;
			}
			catch (const out_of_range& e)
			{
				cerr << "Error: input '" << inputs.
					front() << "' not found in expressions or wires\n"; return -1;
			}
		}

		return result;
	}
};

// Define the Circuit class
class Circuit
{
public:
	unordered_map < string, Component > components;
	unordered_map < string, int >inputs;
	unordered_map < string, int >wires;
	priority_queue < pair < int, string >, vector < pair < int,
		string >>, greater < pair < int, string >>> eventManager;
	// Add components to the circuit and initialize the wire values to 0
	void setInputsValue(const string& input, int value)
	{
		inputs[input] = value;
	}

	void connectCircuit(const Component& component)
	{
		components[component.name] = component; wires[component.output] = 0;	// initialize output wire
	}
	void simulate(priority_queue < pair < int, string >,
		vector < pair < int, string >>,
		greater < pair < int, string >>>& eventManager,
		ofstream& output_file)
	{
		while (!eventManager.empty())
		{
			auto event = eventManager.top();	// Get the top event
			eventManager.pop();	// Correctly remove the top event from the queue
			int timeStamp = event.first;
			string detail = event.second;
			vector < string > wireName; if (detail.substr(0, 6) == "INPUT:")
			{
				size_t equalPos = detail.find("="); if (equalPos != string::npos)
				{
					string inputName = detail.substr(6, equalPos - 6);
					int newValue = stoi(detail.substr(equalPos + 1));
					// Update input value
					inputs[inputName] = newValue;
					// Log input change
					output_file << timeStamp << ", " << inputName << ", " << newValue
						<< endl;
					// For each component, check if it's affected by this input
					for (auto& [compName, comp] : components)
					{

						// Check all inputs are valid before evaluating the component
						bool allInputsValid = true; for (const auto& input : comp.inputs)
						{
							// Here we check existence in wires since inputs should directly map to wires
							if (inputName != input)
							{
								allInputsValid = false; break;
							}
						}

						// Evaluate component output
						int oldValue = wires[comp.output];
						int newValue = comp.evaluateLogic(inputs, wires);;
						// Schedule dependent events
						if (newValue != -1 && newValue != oldValue && allInputsValid)
						{
							wires[comp.output] = newValue;
							output_file << timeStamp +
								comp.delay << ", " << comp.output << ", " << newValue << endl;
							manageOutputEvents(comp.output, timeStamp + comp.delay);
							wireName.push_back(comp.output);
						}

					}

				}
			}
			else
			{


				for (int i = 0; i < wireName.size(); i++)
				{
					// For each component, check if it's affected by this input
					for (auto& [compName, comp] : components)
					{

						// Check all inputs are valid before evaluating the component
						bool allWiresValid = true; for (const auto& wires : comp.inputs)
						{
							// Here we check existence in wires since inputs should directly map to wires
							if (wireName[i] != wires)
							{
								allWiresValid = false; break;
							}
						}


						// Evaluate component output
						int oldValue = wires[comp.output];
						int newValue = comp.evaluateLogic(inputs, wires);
						// Schedule dependent events
						if (newValue != -1 && newValue != oldValue && allWiresValid)
						{
							wires[comp.output] = newValue;
							output_file << timeStamp +
								comp.delay << ", " << comp.output << ", " << newValue << endl;
							manageOutputEvents(comp.output, timeStamp + comp.delay);
						}

					}


					// Component evaluation case, similar logic as above

					Component& comp = components[detail];
					// Check all inputs are valid before evaluating the component
					bool allInputsValid = true; for (const auto& input : comp.inputs)
					{
						// Here we check existence in wires since inputs should directly map to wires
						if (wires.find(input) == wires.end())
						{
							allInputsValid = false; break;
						}
					}

					if (allInputsValid)
					{					// Proceed only if all inputs are valid
					// Evaluate component output
						int oldValue = wires[comp.output];
						int newValue = comp.evaluateLogic(inputs, wires);
						// If output changes
						if (newValue != oldValue)
						{
							wires[comp.output] = newValue;
							// Log the change
							output_file << timeStamp +
								comp.delay << ", " << comp.output << ", " << newValue << endl;
							// Schedule dependent events
							manageOutputEvents(comp.output, timeStamp + comp.delay);
						}
					}
				}
			}
		}
	}

	void manageOutputEvents(const string& updatedWire,
		int currentTime)
	{
		for (auto& [compName, comp] : components)
		{
			// Check if the component is affected by the updated wire
			for (const string& input : comp.inputs)
			{
				if (input == updatedWire)
				{
					// Calculate the time the output will change due to this update
					int scheduledTime = currentTime + comp.delay;
					// Schedule an event to evaluate the component at the calculated time
					eventManager.push(
						{
						scheduledTime, compName });
					// Debug print to log the scheduling
					cout << "Scheduled event for component: " << compName << " at time: " << scheduledTime << endl; break;	// No need to check other inputs for this component
				}
			}
		}
	}
};

// Parse library file
void parseLibraryFile(const string& filename,
	unordered_map < string,
	Component >& libraryComponents)
{
	ifstream file(filename); if (!file)
	{
		cerr << "Unable to open Library file " << filename << endl;
		return;
	}

	string line; while (getline(file, line))
	{
		stringstream ss(line); string componentName; int numInputs, delay; string outputExpression;	// This represents the logic function of the component, not used directly in this context
		ss >> componentName >> numInputs >> outputExpression >> delay;
		// Since the inputs for the component definitions in the library are not directly specified,
		// and the actual inputs will depend on how the component is used in a circuit,
		// we don't create a vector of inputs here. Instead, we're interested in the delay.
		// Check if the component is already in the libraryComponents map
		if (libraryComponents.find(componentName) ==
			libraryComponents.end())
		{
			// Only add the component to the map if it's not already present.
			// This prevents overriding the delay of a component type if it appears more than once in the library file.
			libraryComponents[componentName] =
				Component(componentName, "",
					{
					}
			, "", delay);
		}
	}
	file.close();
}

int getComponentDelay(const string& componentType,
	const unordered_map < string,
	Component >& libraryComponents)
{
	auto it = libraryComponents.find(componentType);
	if (it != libraryComponents.end())
	{
		return it->second.delay;
	}
	return 0;		// Default delay if component type is not found
}

// Parse circuit file 
void parseCircuitFile(const string& filename,
	Circuit& circuit,
	const unordered_map < string,
	Component >& libraryComponents)
{

	ifstream file(filename);
	string line;
	bool isComponentSection = false; while (getline(file, line))
	{
		if (line.find("COMPONENTS:") == 0)
		{
			isComponentSection = true; continue;
		}
		if (!isComponentSection) continue;
		if (line.empty())break;
		stringstream ss(line);
		string componentName, componentType, output;
		vector < string > inputs;
		ss >> componentName >> componentType >> output;
		string token; while (ss >> token)
		{
			inputs.push_back(token);
		}

		// Use the new function to get the component delay
		int delay = getComponentDelay(componentType, libraryComponents); Component component(componentName, componentType, inputs, output, delay); circuit.connectCircuit(component); cout << "Added component: " << componentName << ", Type: " << componentType << ", Output: " << output << ", Inputs: "; for (const auto& input : inputs)
		{
			circuit.inputs[input] = 0; cout << input << " ";
		}
		cout << ", Delay: " << delay << endl;
	}
	file.close();
}




// parse simulation file
void parseSimulate(const string& filename,
	priority_queue < pair < int, string >,
	vector < pair < int, string >>,
	greater < pair < int,
	string >>>& eventManager,
	Circuit& circuit)
{
	ifstream file(filename); if (!file.is_open())
	{
		cerr << "Error: Unable to open stimuli file " << filename <<
			endl; exit(EXIT_FAILURE);
	}

	string line; while (getline(file, line))
	{
		stringstream ss(line);
		int timestamp;
		string input;
		int logic_value; ss >> timestamp >> input >> logic_value;
		// schedule the input value change as an event
		string eventDetail =
			"INPUT:" + input + "=" + to_string(logic_value);
		eventManager.push(make_pair(timestamp, eventDetail));
		cout << "Schedules input change: Time:" << timestamp <<
			",Input" << input << ",New Value:" << logic_value << endl;
	}
	file.close();
}



int main()
{
	string libraryFile = "lib.txt"; string circuitFile = "circuit1.cir"; string stimuliFile = "stimuli.stim"; string simulationOutputFile = "simulation.sim"; unordered_map < string, Component > libraryComponents; parseLibraryFile(libraryFile, libraryComponents); Circuit circuit; parseCircuitFile(circuitFile, circuit, libraryComponents); priority_queue < pair < int, string >, vector < pair < int, string >>, greater < pair < int, string >>> eventManager; parseSimulate(stimuliFile, eventManager, circuit);	// Corrected line
	ofstream simulationOutput(simulationOutputFile);
	if (!simulationOutput.is_open())
	{
		cerr << "Unable to open file for simulation results: " <<
			simulationOutputFile << endl; return -1;
	}

	circuit.simulate(eventManager, simulationOutput);
	simulationOutput.close();
	cout << "Simulation completed. Results are written to " <<
		simulationOutputFile << endl; return 0;
}
