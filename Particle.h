//
// Created by tdolega on 14.04.2021.
//

#ifndef SAND_PARTICLE_H
#define SAND_PARTICLE_H

#include "Renderer.h"

struct Particle {
    types type = T_EMPTY;
    sf::Time lifetime;
    sf::Vector2i velocity{0, 0};
    int hp = 100;

    void setLifetime(sf::Time lifetimeAprox) {
        auto lax = lifetimeAprox.asMilliseconds();
        lifetime = sf::milliseconds((myrand() % lax) * 0.25 + lax);
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
