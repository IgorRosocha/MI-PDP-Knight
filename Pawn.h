//
// Created by Igor R on 3/3/18.
//

#ifndef MI_PDP_JES_PAWN_H
#define MI_PDP_JES_PAWN_H


#include "Square.h"

class Pawn {
private:
    Square square;

public:
    void SetSquare(int, int);
    Square GetSquare();
};


#endif //MI_PDP_JES_PAWN_H
