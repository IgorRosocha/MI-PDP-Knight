# MI-PDP-Knight

This project is a C++ implementation of Knight on chessboard problem (JES), created as a semestral work for **MI-PDP** (Parallel and Distributed Programming) at **FIT CTU**.

Given the size of chessboard and positions of Knight and Pawns, the algorithm finds the shortest Knight path to eliminate all of the pawns. The **Master-Slave** algorithm is parallelized using **OpenMP** and **MPI** libraries to run on a cluster environment (multiple cores/multiple threads).

## How to compile:

Use the following command to compile the program to run parallely:

``mpiCC main.cpp Chessboard.cpp Knight.cpp Pawn.cpp Square.cpp -o JES -O3 -lstdc++ -mavx -std=c++11 -g -fopenmp``

## Usage:

Use the following command to run the program parallely:

``./JES input_file.txt``

*Note: You can see the example input files in the examples folder.*

## License
This project is licensed under the **MIT License**.
