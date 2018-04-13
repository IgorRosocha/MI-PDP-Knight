//
// Created by Igor R on 3/1/18.
//

#include <deque>
#include "Knight.h"

bool Knight::IsNextValid(int nextX, int nextY, int size)
{
    if (nextX < size && nextY < size && nextX >= 0 && nextY >= 0)
    {
        return true;
    }
    return false;
}

void Knight::SetSquare(int x, int y)
{
    //square.SetSquare(this->x, this->y);
    this->square.SetSquare(x, y);
    //this->square = square;
}

Square Knight::GetSquare() { return this->square; }

std::deque<Square> Knight::PossibleJumps(int x, int y, int size){
    std::deque<Square> nextSquaresList;
    Square nextSquare;

    if (IsNextValid(x + 2, y + 1, size)){
        nextSquare.SetSquare(x + 2, y + 1);
        nextSquaresList.push_back(nextSquare);
    }

    if (IsNextValid(x + 2, y - 1, size)){
        nextSquare.SetSquare(x + 2, y - 1);
        nextSquaresList.push_back(nextSquare);
    }

    if (IsNextValid(x + 1, y + 2, size)){
        nextSquare.SetSquare(x + 1, y + 2);
        nextSquaresList.push_back(nextSquare);
    }

    if (IsNextValid(x + 1, y - 2, size)){
        nextSquare.SetSquare(x + 1, y - 2);
        nextSquaresList.push_back(nextSquare);
    }

    if (IsNextValid(x - 1, y + 2, size)){
        nextSquare.SetSquare(x - 1, y + 2);
        nextSquaresList.push_back(nextSquare);
    }

    if (IsNextValid(x - 1, y - 2, size)){
        nextSquare.SetSquare(x - 1, y - 2);
        nextSquaresList.push_back(nextSquare);
    }

    if (IsNextValid(x - 2, y + 1, size)){
        nextSquare.SetSquare(x - 2, y + 1);
        nextSquaresList.push_back(nextSquare);
    }

    if (IsNextValid(x - 2, y - 1, size)){
        nextSquare.SetSquare(x - 2, y - 1);
        nextSquaresList.push_back(nextSquare);
    }

    return nextSquaresList;
}