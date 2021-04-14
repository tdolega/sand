//
// Created by tdolega on 14.04.2021.
//

#ifndef SAND_CHUNKWORKER_H
#define SAND_CHUNKWORKER_H

#include "Map.h"

class ChunkWorker {
    Map *m_map;
    Chunk *m_chunk;

public:
    ChunkWorker(Map *&map, Chunk *&chunk) :
            m_map(map), m_chunk(chunk) {};

    void update(const sf::Time &elapsed) {
        for (int i = 0; i < m_chunk->m_CI; i++) {
            updateParticle(elapsed, i);
        }
    }

    void commit() {
        m_chunk->commitMoves();
    }

    void updateParticle(const sf::Time &elapsed, int idx) {
        auto *p = m_chunk->getParticle(idx);
        if (p->type == T_EMPTY || p->type == T_STONE) return;

        int x, y;
        m_chunk->getXY(idx, x, y);
        auto nx = x, ny = y;
        if (getNextPosition(p, nx, ny)) {
            if (m_chunk->inBounds(nx, ny)) {
                int nidx = m_chunk->getParticleIdx(nx, ny);
                m_chunk->scheduleMove(nidx, idx);
            } else {
                m_map->getChunk(nx, ny);
                //todo
            }
        } else {
            p->velocity.x = 0;
            p->velocity.y = 0;
        }
    }


    bool getNextPosition(Particle *&p, int &nx, int &ny) const {
        Particle *np = nullptr;
        int ty, tx, x = nx, y = ny;
        unsigned mm = p->moveMask();

        auto exists = [&](const int x, const int y) {
            if(!m_map->inBounds(x, y)) return false;
            np = m_map->getParticle(x, ty);
            return true;
        };
        auto canSwap = [&]() {
            return (np->type == T_EMPTY
                    || ((mm & MOVES_FORCE)
                        && np->type != T_STONE
                        && np->type != p->type));
        };
        auto goodMove = [&](const int x, const int y) {
            if(!exists(x, y) || !canSwap()) return false;
            nx = x;
            ny = y;
            return true;
        };

        if (mm & MOVES_D) {
            ty = y + 1;
            if (goodMove(x, ty)) return true;
        }
        if (mm & MOVES_LR) {
            int direction = p->velocity.x
                            ? (p->velocity.x & -1)
                            : (myrand() % 2) ? 1 : -1;
            tx = nx + direction;
            if (goodMove(tx, y)) return true;
            tx = x - direction;
            if (goodMove(tx, y)) return true;
        }
        if (mm & MOVES_LRD) {
            int direction = p->velocity.x
                            ? (p->velocity.x & -1)
                            : (myrand() % 2) ? 1 : -1;
            tx = nx + direction;
            ty = y + 1;
            if (goodMove(tx, ty)) return true;
            tx = nx - direction;
            if (goodMove(tx, ty)) return true;
        }
        return false;
    }
};

#endif //SAND_CHUNKWORKER_H
