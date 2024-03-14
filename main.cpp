#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <queue>

using namespace std;





// Define the Circuit class
class Circuit
{
public:
    unordered_map<string, Component> components;
    unordered_map<string, int> inputs;
    unordered_map<string, int> wires;
    vector<Component> components;

    // Add components to the circuite and initialize the wire values to 0
    void setInputsValue(string input, int value)
    {
        inputs[input] = value;
    }
    
    void connectCircuite(Component& component)
    {
        components[component.name] = component;
        wires[component.name] = 0;
    }
  
    // Function to simulate the circuit
    void simulate(priority_queue<pair<int, string>>& eventManager,ofstream& output_file)
    {
        while (!eventManager.empty()) 
        {
            auto event = eventManager.top(); //auto -> to automatically determine data type
            eventManager.pop();

            int timeStamp = event.first; //accessing the first element of the pair event.
            string target = event.second;

            if (target[0] == 'I') // for input events
            {
                string input = target.substr(1); // extracting a substring starting from the second character -> i1
                int value = inputs[input];
                if (value != wires[target])  // to change the wire input value
                {
                    wires[target] = value;
                    output_file << timeStamp << ", " << target << ", " << value << "\n";
                    cout << "Log:\nInput event: " << timeStamp << ", " << target << ", " << value << "\n"; //log for debugging
                    manageOutputEvents(target, eventManager, timeStamp);
                }
            }
            else
            {
                Component& component = components[target];
                int value = component.evaluateLogic(wires); //the evaluate function from the component class
                if (value != wires[target]) {
                    wires[target] = value;
                    output_file << timeStamp << ", " << target << ", " << value << endl;
                    cout << "Component event: " << timeStamp << ", " << target << ", " << value << endl; // Debug output
                    manageOutputEvents(target, eventManager, timeStamp);
                }
            }

        }


    }

    void manageOutputEvents(string wire, priority_queue<pair<int, string>>& eventManager, int timeStamp) {
        for (auto& event : components) 
        {
            Component& component = event.second;
            if (component.type != "INPUT" && component.type != "NOT")
            {
                for (string input : component.inputs) 
                {
                    if (input == wire) 
                    {
                        eventManager.push(make_pair(timeStamp + component.delay, component.name));
                    }
                }
            }
        }
    }


};




