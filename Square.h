//
// Created by Igor R on 3/3/18.
//

#ifndef MI_PDP_JES_SQUARE_H
#define MI_PDP_JES_SQUARE_H


#include <list>
#include <climits>

using namespace std;

class Square {
private:
    int x, y;
    int value = 0;
    int pawnDistance = INT_MAX;

public:
    void SetSquare(int, int);
    void SetValue(int);
    void SetPawnDistance(list<Square>);
    int GetX();
    int GetY();
    int GetValue();
    int GetPawnDistance();

};


#endif //MI_PDP_JES_SQUARE_H
