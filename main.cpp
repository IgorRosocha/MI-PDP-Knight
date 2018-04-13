#include <iostream>
#include <fstream>
#include <deque>
#include "Knight.h"
#include "Chessboard.h"
#include "Pawn.h"


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
    cout << "[CHESSBOARD SIZE]: " << size << "x" << size << endl;
    cout << "[UPPER LIMIT]: " << UPPER_LIMIT << endl;

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
    for (auto it = pawnsAlive.begin(); it != pawnsAlive.end(); ++it){
        if (recursion_knight->GetSquare().GetX() == it->GetX() && recursion_knight->GetSquare().GetY() == it->GetY()) {
            pawnsAlive.erase(it);
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
                    cout << "New optimal price found: " << OPTIMAL_PRICE - 1 << endl;
                }
            }
        }
        return;
    }

    //get the list of possible jumps of Knight
    deque<Square> possibleJumps = recursion_knight->PossibleJumps(recursion_knight->GetSquare().GetX(), recursion_knight->GetSquare().GetY(), chessboard->GetSize());

    //recursively run through all possible jumps
    for (Square jump : possibleJumps){
            SolveDFS(depth + 1, path, jump.GetX(), jump.GetY(), pawnsAlive);
    }
}

// Main method.
int main(int argc, char* argv[]) {
    knight = new Knight();
    chessboard = new Chessboard();
    deque<Square> path;
    string file;

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

    // Fill the queue with starting possible moves/jumps.
    deque<Square> queue = FillQueue(knight);
    //cout << "KNIGHT:" << knight->GetSquare().GetX() << knight->GetSquare().GetY() << "\n";

    // Start the execution time stopwatch
    clock_t tStart = clock();

    // Solve the problem using data parallelism - inside parallel for loop.
    int i = 0;
    #pragma omp parallel for default(shared) private(i)
    for (i=0; i < queue.size(); i++) {
        SolveDFS(queue.at(i).GetDepth(), path, queue.at(i).GetX(), queue.at(i).GetY(), pawnsList);
    }

    // Print the execution time of algorithm, optimal price obtained and the optimal path
    cout << "------------------------------" << endl;
    printf("Execution time: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    cout << "OPTIMAL PRICE: "<< OPTIMAL_PRICE -1 << endl;
    cout << "PATH: ";
    for (auto p : OPTIMAL_PATH)
        cout << "(" << p.GetX() << "," << p.GetY() << ") ";
    cout << "\n";

    return 0;
}