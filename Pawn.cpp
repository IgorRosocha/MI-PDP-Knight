//
// Created by Igor R on 3/3/18.
//

#include "Pawn.h"

void Pawn::SetSquare(int x, int y)
{
    this->square.SetSquare(x, y);
}

Square Pawn::GetSquare() { return this->square; }
