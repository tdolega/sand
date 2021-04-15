//
// Created by tdolega on 14.04.2021.
//

#ifndef SAND_VARIABLES_H
#define SAND_VARIABLES_H

#include "importsxd.h"

//const int W = 2400;
//const int H = 1300;
//int brushSize = 80;

//const int W = 600;
//const int H = 300;
//int brushSize = 20;
//const int PIXEL_SIZE = 2;

const int W = 400;
const int H = 200;
int brushSize = 3;
const int PIXEL_SIZE = 4;

//const int CHUNKSIZEX = 400;
const int CHUNKSIZEX = 100;
const int CHUNKSIZEY = 100;
//const int W = 1200;
//const int H = 600;
//int brushSize = 20;
//const int PIXEL_SIZE = 1;

//const float LIFETIME_MS = 50000;
const int FILL_PERC = 1;
const int ITHREADS = 4;


//const sf::Time LIFETIME = sf::seconds(LIFETIME_MS);

enum types { // color and id
    T_EMPTY = 0,
    T_SAND = 0xc2b280ff,
    T_WATER = 0x2389daff,
    T_STONE = 0x888c8dff,
    T_LAVA,
    T_FIRE,
    T_SMOKE,
    T_SANDPINK = 0xe324d6ff,
};

enum moves {
    MOVES_NONE = 0b000000,
    MOVES_U = 0b000001,
    MOVES_LR = 0b000010,
    MOVES_D = 0b000100,
    MOVES_LRD = 0b001000,
    MOVES_LRU = 0b010000,
    MOVES_FORCE = 0b100000,
};

#endif //SAND_VARIABLES_H
