//
// Created by Igor R on 3/3/18.
//

#include <iostream>
#include "Square.h"


void Square::SetSquare(int x, int y) {
    this->x = x;
    this->y = y;
}

void Square::SetValue(int value)
{
    this->value = value;
}

int Square::GetPawnDistance(){
    return this->pawnDistance;
}

void Square::SetPawnDistance(list<Square> pawns){
    for (auto it = pawns.begin(); it != pawns.end(); ++it){
        this->pawnDistance = abs(this->GetX() - pawns.get_allocator().address(*it)->GetX()) +
                abs(this->GetY() - pawns.get_allocator().address(*it)->GetY());
    }
}

void Square::SetDepth(int dpth) {
    this->sDepth = dpth;
}

int Square::GetX() { return x; }
int Square::GetY() { return y; }
int Square::GetValue() { return value; }
int Square::GetDepth() { return sDepth; }
