#include <iostream>
#include <fstream>
#include "Knight.h"
#include "Chessboard.h"
#include "Pawn.h"


Knight* knight;
Chessboard* chessboard;
list<Square> pawnsList;
int UPPER_LIMIT = 0;
int OPTIMAL_PRICE = INT_MAX;
list<Square> OPTIMAL_PATH;

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

void SolveDFS(int depth, list<Square> path, int newX, int newY, list<Square> pawnsAlive){
    //end the branch if upper limit depth is reached
    //or if it isn't possible to get a better price
    if (depth > UPPER_LIMIT || depth + pawnsAlive.size() > UPPER_LIMIT || depth + pawnsAlive.size() >= OPTIMAL_PRICE){
        return;
    }

    //set the new position of Knight and put it into the path list
    knight->SetSquare(newX, newY);
    path.push_back(knight->GetSquare());

    //erase the pawn if it has been eliminated by the Knight
    for (auto it = pawnsAlive.begin(); it != pawnsAlive.end(); ++it){
        if (knight->GetSquare().GetX() == it->GetX() && knight->GetSquare().GetY() == it->GetY()) {
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
    list<Square> possibleJumps = knight->PossibleJumps(knight->GetSquare().GetX(), knight->GetSquare().GetY(), chessboard->GetSize());

    //set the Knight's distance from every pawn
    //and set pawns squares value to 1
    for (auto jumpsIt = possibleJumps.begin(); jumpsIt != possibleJumps.end(); ++jumpsIt) {
        possibleJumps.get_allocator().address(*jumpsIt)->SetPawnDistance(pawnsAlive);
        for(Square pawn : pawnsAlive){
            if (jumpsIt->GetX() == pawn.GetX() && jumpsIt->GetY() == pawn.GetY())
                possibleJumps.get_allocator().address(*jumpsIt)->SetValue(1);
        }
    }

    //sort the list of possible jumps based on the given heuristic (effectivity of jump)
    possibleJumps.sort([](Square & a, Square & b) { return (8*(a.GetValue()) - a.GetPawnDistance()) > (8*(b.GetValue()) - b.GetPawnDistance()); });

    //recursively run through all possible jumps
    for (Square jump : possibleJumps){
        //if actual path is shorter than 4, run recursion as a parallel task, otherwise run recursion sequentially
        if (path.size() < 4){
            #pragma omp task
            SolveDFS(depth + 1, path, jump.GetX(), jump.GetY(), pawnsAlive);
        } else {
            SolveDFS(depth + 1, path, jump.GetX(), jump.GetY(), pawnsAlive);
        }
    }
}


int main(int argc, char* argv[]) {
    knight = new Knight();
    chessboard = new Chessboard();
    list<Square> path;
    string file;

    //specify the input data file as a command line argument
    if (argc > 1) {
        file = argv[1];
    } else {
        cout << "Please specify the input data file!" << endl;
        return 10;
    }

    //read the input from the data file
    ReadFile("examples/" + file);

    //start the execution time stopwatch
    clock_t tStart = clock();

    #pragma omp parallel
    {
    	//start the recursion with the initial position of Knight inside a parallel region on a single thread
    	#pragma omp single
        	SolveDFS(0, path, knight->GetSquare().GetX(), knight->GetSquare().GetY(), pawnsList);
    }

    //print the execution time of algorithm, optimal price obtained and the optimal path
    cout << "------------------------------" << endl;
    printf("Execution time: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    cout << "OPTIMAL PRICE: "<< OPTIMAL_PRICE -1 << endl;
    cout << "PATH: ";
    for (auto p : OPTIMAL_PATH)
        cout << "(" << p.GetX() << "," << p.GetY() << ") ";
    cout << "\n";

    return 0;
}