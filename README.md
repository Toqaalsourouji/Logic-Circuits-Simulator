# Logic-Circuit-Simulator-DD1

## Group Members:
1. Toqa Mahmoud Hassanien
2. Adham Gohar
3. Khadeejah Iraky

## Running the Code:
To build and use this project you need to download all the given files in the project, most importantly the following files; `main.cpp`, `GateSimulator.h`, `GateSimulator.cpp`, `Event.h`, and `Gate.h`. 
You can create your own library file to define your own gates as long as it follows a structure like the library.lib or the structure below:

GATE_NAME, no._of_inputs[^1], the_expression_of_the_gates[^2], delay*ns
[^1]: (as an integer)
[^2]: (using i1, i2, ...., in for inputs, and ~, |, &, ^ as operations, operations have to be within inputs)

Here are a few examples:
```
OR, 2, i1|i2, 100ns
NAND, 3, ~(i1&i2&i3), 150ns
MUX, 3, (i1&(~i3))|(i2&i3), 300ns
```


> [!NOTE]
> or you can use the library file provided in the repo.

You can also create your own circuit file by following the below example:
```
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
```

defining all used inputs under INPUTS:, and then defining the gates under COMPONENTS:, using the below format:

gate_label[^3], GATE_NAME[^4], output[^5], inputs[^6]

[^3]: as G0, G1, ..., Gn
[^4]: as defined in the library file
[^5]: mostly defined as wires, expressed as; W1, W2, ..., Wn
[^6]: inputs as defined in the INPUTS: section

> [!NOTE]
> or you can use the circuit files provided in this repo.

If you created your own circuit file, you will need to create an accompanying simulation file.

After having the necessary files downloaded, you will right-click in the folder with the files, press more options and then click 'git bash here', and then write the following commands:


