#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

class Component {
public:
    string name;
    string type;
    string output;
    vector<string> inputs;
    int delay;

    Component(string n, string t, string o, vector<string> in, int d)
        : name(n), type(t), output(o), inputs(in), delay(d) {}
};

int readLib(vector<Component>& myComponent, string filename)
{
   
    string thing;

    ifstream inputfile;
    inputfile.open(filename); // Opening the file

    if (!inputfile.is_open())
    {
        cout << "Cannot find input file" << endl;
        return 0;
    }

    while (inputfile.eof() == false)
    {
        thing = "";
        getline(inputfile, thing);

        std::string gateName = thing.substr(0, thing.find(','));
        thing = thing.substr(thing.find(',') + 1, thing.length());

        std::string gateType = thing.substr(0, thing.find(','));
        thing = thing.substr(thing.find(',') + 1, thing.length());

        std::string gateOutput = thing.substr(0, thing.find(','));
        thing = thing.substr(thing.find(',') + 1, thing.length());

        int i = 0;
        bool t = true;

        while (t) 
        {
            t = thing.find(',');
            
            std::string gateInput = thing.substr(0, thing.find(','));
            thing = thing.substr(thing.find(',') + 1, thing.length());

            myComponent[i].push_back(gateInput);
            i++;
        }

        std::string gateInput = thing;
        myComponent[i].push_back(gateInput);

        myComponent.name = gateName;
        myComponent.type = gateType;
        myComponent.output = gateOutput;
        myComponent
        
    }
    inputfile.close();

    return 0;
}

int main()
{
    
}
