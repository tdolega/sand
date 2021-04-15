//
// Created by tdolega on 14.04.2021.
//

#ifndef SAND_PARTICLE_H
#define SAND_PARTICLE_H

#include "Renderer.h"


enum types { // color and id
    T_EMPTY = 0,
    T_SAND = 0xc2b280ff,
    T_WATER = 0x2389daff,
    T_STONE = 0x888c8dff,
    T_LAVA,
    T_FIRE,
    T_SMOKE = 0x3b3b38ff,
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

struct Particle {
    types type;
    sf::Time lifetime;
    sf::Vector2i velocity{0, 0};
    int hp = 100;

    Particle(const types type = T_EMPTY) : type(type) {};

    void setLifetime(sf::Time lifetimeAprox) {
        auto lax = lifetimeAprox.asMilliseconds();
        lifetime = sf::milliseconds((random_st() % lax) * 0.25 + lax);
    }

    [[nodiscard]] unsigned moveMask() const {
        switch (type) {
            case T_SAND:
            case T_SANDPINK:
                return MOVES_D | MOVES_LRD | MOVES_FORCE;
            case T_WATER:
                return MOVES_D | MOVES_LRD | MOVES_LR;
            case T_SMOKE:
                return MOVES_U | MOVES_LRU | MOVES_LR;
            default:
                return MOVES_NONE;
        }
    }
};

#endif //SAND_PARTICLE_H
