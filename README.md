# Logic-Circuit-Simulator-DD1

## Group Members:
1. Toqa Mahmoud Hassanien
2. Adham Gohar
3. Khadeejah Iraky

## Running the Code:
To build and use this project you need to download all the given files in the project, most importantly the following files; main.cpp, GateSimulator.h, GateSimulator.cpp, Event.h, and Gate.h. 
You can create your own library file to define your own gates as long as it follows a structure like the library.lib or the structure below:

GATE_NAME, no._of_inputs*, the_expression_of_the_gates**, delay*ns
* (as an integer)
** (using i1, i2, ...., in for inputs, and ~, |, &, ^ as operations, operations have to be within inputs)

Here are a few examples:
  OR, 2, i1|i2, 100ns
  NAND, 3, ~(i1&i2&i3), 150ns
  MUX, 3, (i1&(~i3))|(i2&i3), 300ns

or you can use the library file provided in the repo.

You can also create your own circuit file by following the below example:

INPUTS:
A
B
C
D
COMPONENTS:
G0, OR2, W1, C, B
G1, NOT, W2, W1 
G2, AND2, W3, W1, D
G3, AND2, W4, A, W2
G4, OR2, Y, W4, W3

defining all used inputs under INPUTS:, and then d

or you can use the circuit files provided in this repo.

right click get bash here and then write the following commands:
