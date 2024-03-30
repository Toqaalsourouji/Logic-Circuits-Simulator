#include "GateSimulator.h"
#include "Event.h"
#include "Gate.h"

using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cerr << "Usage: " << argv[0] << " <library file> <circuit file> <stimuli file>" << endl;
		return 1;
	}

	GateSimulator simulator(argv[1], argv[2], argv[3]); // // Creates a GateSimulator object with the provided file paths
	simulator.startSimulation(); // Starts the simulation process

	return 0;
}
