#include <iostream>
#include <fstream>
#include <deque>
#include <mpi.h>
#include <thread>
#include "Knight.h"
#include "Chessboard.h"
#include "Pawn.h"

#define TAG_WORK 10
#define TAG_WAITING 11
#define TAG_DONE 12
#define TAG_FINISHED 13


Knight* knight;
Chessboard* chessboard;
list<Square> pawnsList;
int UPPER_LIMIT = 0;
int OPTIMAL_PRICE = INT_MAX;
deque<Square> OPTIMAL_PATH;

// Reads chessboard information (size, knight and pawns positions) from the input file.
void ReadFile(const string file_path){
    ifstream file(file_path);

    if(!file){
        cerr << "Specified file does not exist!\n";
        exit(10);
    }

    int size;
    file >> size >> UPPER_LIMIT;
    chessboard->SetSize(size);

    string line;
    for (int i=0; i < size; i++){
        file >> line;
        for (int j=0; j < size; j++){
            if (line[j] == '1') {
                auto pawn = new Pawn();
                pawn->SetSquare(i, j);
                pawnsList.push_back(pawn->GetSquare());
            } else if (line[j] == '3'){
                knight->SetSquare(i, j);
            }
        }
    }
}

// Fills the deque with the given number of possible moves/jumps.
deque<Square> FillQueue(Knight* initial_knight){
    deque<Square> knights;
    Knight* current_knight;
    int depth_index = 0;

    current_knight = initial_knight;
    initial_knight->GetSquare().SetDepth(0);
    knights.push_front(initial_knight->GetSquare());

    while (knights.size() < 20) {
        current_knight->SetSquare(knights.front().GetX(), knights.front().GetY());
        deque<Square> jumps = current_knight->PossibleJumps(current_knight->GetSquare().GetX(),
                                                           current_knight->GetSquare().GetY(),
                                                           chessboard->GetSize());
        depth_index++;

        for (Square jump : jumps) {
            jump.SetDepth(depth_index);
            knights.push_back(jump);
        }
    }
    return knights;
}

// DFS recursive method to solve the given problem (JES).
void SolveDFS(int depth, deque<Square> path, int newX, int newY, list<Square> pawnsAlive){
    //end the branch if upper limit depth is reached
    //or if it isn't possible to get a better price
    if (depth > UPPER_LIMIT || depth + pawnsAlive.size() > UPPER_LIMIT || depth + pawnsAlive.size() >= OPTIMAL_PRICE){
        return;
    }

    //set the new position of Knight and put it into the path list
    auto recursion_knight = new Knight();
    recursion_knight->SetSquare(newX, newY);
    path.push_back(recursion_knight->GetSquare());

    //erase the pawn if it has been eliminated by the Knight
    for (auto it = pawnsAlive.begin(); it != pawnsAlive.end();){
        if (recursion_knight->GetSquare().GetX() == it->GetX() && recursion_knight->GetSquare().GetY() == it->GetY()) {
            pawnsAlive.erase(it++);
        } else {
            ++it;
        }
    }

    //if all pawns are eliminated, and path size is smaller than OPTIMAL price, set the new OPTIMAL price
    if (pawnsAlive.empty()){
        if (path.size() < OPTIMAL_PRICE) {
            //critical section - a part of a parallel block that can only performed by one thread at a time
            #pragma omp critical
            {
                if (path.size() < OPTIMAL_PRICE) {
                    OPTIMAL_PRICE = path.size();
                    OPTIMAL_PATH = path;
                }
            }
        }
        return;
    }

    //get the list of possible jumps of Knight
    deque<Square> possibleJumps = recursion_knight->PossibleJumps(recursion_knight->GetSquare().GetX(), recursion_knight->GetSquare().GetY(), chessboard->GetSize());

    //recursively run through all possible jumps
    for (Square jump : possibleJumps){
        if (path.size() < 4) {
            #pragma omp task
            SolveDFS(depth + 1, path, jump.GetX(), jump.GetY(), pawnsAlive);
        } else {
            SolveDFS(depth + 1, path, jump.GetX(), jump.GetY(), pawnsAlive);
        }
    }
}

// Main method.
int main(int argc, char* argv[]) {
    knight = new Knight();
    chessboard = new Chessboard();
    deque<Square> path;
    deque<Square> queue;
    string file;
    auto *square = new Square();

    // MPI related variables
    int rank, num_procs, flag;
    MPI_Status status;
    int working = 0;
    int state_size = 0;
    int provided, required = MPI_THREAD_MULTIPLE;

    // Specify the input data file as a command line argument
    if (argc > 1) {
        file = argv[1];
    } else {
        cout << "Please specify the input data file!" << endl;
        return 10;
    }

    // Read the input from the data file
    ReadFile("examples/" + file);

    // Push the starting position of the knight into the path.
    path.push_back(knight->GetSquare());

    // Start up MPI with THREAD_MULTIPLE (if available)
    MPI_Init_thread(&argc, &argv, required, &provided);
    if (provided < required) {
        throw std::runtime_error("MPI library does not provide required threading support! (MPI_THREAD_MULTIPLE)");
    }

    // Find out the process rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Find out number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Start the execution time stopwatch
    clock_t tStart = clock();

    // Master process
    if (rank == 0){
        // Fill the queue with initial states
        queue = FillQueue(knight);

        // While there are still states left in queue, send them to available slaves waiting for work
        while (!queue.empty()) {
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == TAG_WAITING || status.MPI_TAG == TAG_DONE) {
                cout << "[MASTER] Sending data to slave number " << status.MPI_SOURCE << endl;
                MPI_Send(&queue.front(), sizeof(queue.front()), MPI_BYTE, status.MPI_SOURCE, TAG_WORK,
                         MPI_COMM_WORLD);
                queue.pop_front();
                working++;
                }

            // If there is (are) working slave(s), wait for their results
            if (working > 0){
                MPI_Probe(MPI_ANY_SOURCE, TAG_DONE, MPI_COMM_WORLD, &status);
                MPI_Recv(&OPTIMAL_PRICE, sizeof(int), MPI_INT, status.MPI_SOURCE, TAG_DONE, MPI_COMM_WORLD, &status);
                working--;
                cout << "[MASTER] Received the result from slave " << status.MPI_SOURCE << endl;
            }
        }

        // If there are no states left in queue, acknowledge the slaves
        for (int dest = 1; dest < num_procs; dest++){
            MPI_Send(nullptr, 0, MPI_SHORT, dest, TAG_FINISHED, MPI_COMM_WORLD);
        }
        MPI_Probe(MPI_ANY_SOURCE, TAG_FINISHED, MPI_COMM_WORLD, &status);

        // Shut down MPI if all the slaves have finished the work
        if (status.MPI_TAG == TAG_FINISHED){
            MPI_Recv(nullptr, 0, MPI_SHORT, status.MPI_SOURCE, TAG_FINISHED, MPI_COMM_WORLD, &status);
            cout << "[MASTER] Received FINISH TAG from slaves, finalizing MPI.." << endl;
            cout << "------------------------------\n" << endl;
            cout << "[SUMMARY]" << endl;
            printf("Execution time: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
            cout << "OPTIMAL PRICE: "<< OPTIMAL_PRICE -1 << endl;
        }

    // Slave process
    } else {
        // Acknowledge the master that I'm ready and waiting for work
        MPI_Send(nullptr, 0, MPI_BYTE, 0, TAG_WAITING, MPI_COMM_WORLD);
        cout << "[SLAVE] " << rank << ": Sent WAITING tag to MASTER and waiting"
                                      " for work to be assigned.." << endl;
        bool end = false;

        // While there is still work to do
        while (!end) {
            MPI_Iprobe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

            // If there is a message to receive
            if (flag) {
                // Get the size of message
                MPI_Get_count(&status, MPI_BYTE, &state_size);

                // If the tag of message received from master is WORK, receive the state and solve the problem
                // (using OpenMP task parallelism), then send the result to the master
                if (status.MPI_TAG == TAG_WORK) {
                    MPI_Recv(square, state_size, MPI_BYTE, 0, TAG_WORK, MPI_COMM_WORLD, &status);
                    cout << "[SLAVE] " << rank << ": I have received a state of length: " << state_size << endl;
                    #pragma omp parallel
                    {
                        #pragma omp single
                        SolveDFS(square->GetDepth(), OPTIMAL_PATH, square->GetX(), square->GetY(),
                                 pawnsList);
                    }

                    // Notify master that I've DONE my job and I'm ready for another one to be assigned
                    cout << "[SLAVE] " << rank << ": I have finished my work and"
                                                  " I'm sending result to MASTER..\n";
                    MPI_Send(&OPTIMAL_PRICE, sizeof(int), MPI_INT, 0, TAG_DONE, MPI_COMM_WORLD);

                // If master notified me that all the work has been finished
                // (there are no states left in master queue), send the FINISHED tag to master
                } else {
                    cout << "[SLAVE] MASTER notified me that everything is FINISHED.." << endl;
                    end = true;
                    MPI_Send(nullptr, 0, MPI_SHORT, 0, TAG_FINISHED, MPI_COMM_WORLD);
                }
            }
        }
    }

    MPI_Finalize();
    return 0;
}