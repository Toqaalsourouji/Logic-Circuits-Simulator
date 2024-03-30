#include "CircuitSimulator.h"
#include "Event.h"
#include "Gate.h"

using namespace std;

int main(int argc, char* argv[])
{
    // Check if the correct number of arguments is passed to the program.
    // Expecting 3 arguments: library file, circuit file, and stimuli file.
    if (argc != 4)
    {
        // If the incorrect number of arguments is provided, print usage instructions to the standard error stream.
        // This helps users understand how to correctly execute the program.
        cerr << "Usage: " << argv[0] << " <library file> <circuit file> <stimuli file>" << endl;
        // Exit the program with a non-zero status to indicate an error.
        return 1;
    }

    // Create an instance of the CircuitSimulator class, passing in the file paths provided by the user.
    // This object will manage the setup and execution of the circuit simulation.
    CircuitSimulator simulator(argv[1], argv[2], argv[3]);
    // Start the simulation process. This function call initiates the reading of the input files,
    // sets up the simulation environment, and begins processing events.
    simulator.startSimulation();

    return 0;
}
