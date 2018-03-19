//
// Created by Igor R on 3/1/18.
//

#ifndef MI_PDP_JES_KNIGHT_H
#define MI_PDP_JES_KNIGHT_H


#include "Square.h"

class Knight {
private:
    Square square;

public:
    void SetSquare(int, int);
    Square GetSquare();
    bool IsNextValid(int nextX, int nextY, int size);
    list<Square> PossibleJumps(int x, int y, int size);


};


#endif //MI_PDP_JES_KNIGHT_H
