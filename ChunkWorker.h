//
// Created by tdolega on 14.04.2021.
//

#ifndef SAND_CHUNKWORKER_H
#define SAND_CHUNKWORKER_H

#include "Map.h"

class ChunkWorker {
    Map &m_map;
    Chunk &m_chunk;

public:
    ChunkWorker(Map &map, Chunk &chunk) :
            m_map(map), m_chunk(chunk) {};

    void update(const sf::Time &elapsed) {
        for (int i = 0; i < m_chunk.m_CI; i++) {
            updateParticle(elapsed, i);
        }
    }

    Particle* getNextPosition(const Particle &p, int &x, int &y) {
        Particle* np;
        int ty, tx;
        unsigned mm = p.moveMask();

        auto exists = [&](const int nx, const int ny) {
            if(!m_map.inBounds(nx, ny)) return false;
            np = &m_map.getParticle(nx, ny);
            return true;
        };
        auto canSwap = [&]() {
            return (np->type == T_EMPTY
                    || ((mm & MOVES_FORCE)
                        && np->type != T_STONE
                        && !(np->moveMask() & MOVES_FORCE)));
        };
        auto goodMove = [&](const int nx, const int ny) {
            if(!exists(nx, ny) || !canSwap()) return false;
            x = nx;
            y = ny;
            return true;
        };

        if (mm & MOVES_D) {
            ty = y + 1;
            if (goodMove(x, ty)) return np;
        }
        if (mm & MOVES_U) {
            ty = y - 1;
            if (goodMove(x, ty)) return np;
        }

        int direction = p.velocity.x
                        ? (p.velocity.x & -1)
                        : (random_mt() % 2) ? 1 : -1;
        if (mm & MOVES_LR) {
            tx = x + direction;
            if (goodMove(tx, y)) return np;
            tx = x - direction;
            if (goodMove(tx, y)) return np;
        }
        if (mm & MOVES_LRD) {
            tx = x + direction;
            ty = y + 1;
            if (goodMove(tx, ty)) return np;
            tx = x - direction;
            if (goodMove(tx, ty)) return np;
        }
        if (mm & MOVES_LRU) {
            tx = x + direction;
            ty = y - 1;
            if (goodMove(tx, ty)) return np;
            tx = x - direction;
            if (goodMove(tx, ty)) return np;
        }
        return nullptr;
    }

    void updateParticle(const sf::Time &elapsed, const int idx) {
        auto &p = m_chunk.getParticle(idx);
        if (p.type == T_EMPTY || p.type == T_STONE) return;

        const auto [x, y] = m_chunk.getXY(idx);
        auto nx = x, ny = y;
        if (Particle* np = getNextPosition(p, nx, ny)) {
            auto &c = m_chunk.inBounds(nx, ny) ? m_chunk : m_map.getChunk(nx, ny);
            int gidx = m_map.getParticleGIdx(x, y);
            int ngidx = m_map.getParticleGIdx(nx, ny);
            c.scheduleMove(ngidx, gidx);
        } else {
            p.velocity.x = 0;
            p.velocity.y = 0;
        }
    }

    void commit() {
        auto &swaps = m_chunk.m_swaps;
//        std::sort(std::execution::par_unseq, swaps.begin(), swaps.end(),
        std::sort(swaps.begin(), swaps.end(),
                  [](auto &a, auto &b) { return a.first < b.first; }
        );

        swaps.emplace_back(-1, -1); // catch final move

        int iprev = 0;
        for (int i = 0; i < swaps.size() - 1; i++) {
            if (swaps[i + 1].first != swaps[i].first) {
                int rand = iprev + random_mt() % (i - iprev + 1);
                auto [didx, sidx] = swaps[rand];
                int dx = didx%W, dy = didx/W;
                int sx = sidx%W, sy = sidx/W;
                Particle &dp = m_chunk.getParticle(dx, dy);
                Particle &sp = m_map.getParticle(sx, sy);
                dp.velocity.x = sx - dx;
                dp.velocity.y = sy - dy;
                sp.velocity.x = dx - sx;
                sp.velocity.y = dy - sy;
                std::swap(dp, sp);
                renderer.updateVertex(didx, dp.type);
                renderer.updateVertex(sidx, sp.type);
                iprev = i + 1;
            }
        }
        swaps.clear();
    }

    uint64_t wyhash64_x_mt;
    uint64_t random_mt() {
//        return random_st();
//        return 1;
//        return rand();
        wyhash64_x_mt += 0x60bee2bee120fc15;
        __uint128_t tmp;
        tmp = (__uint128_t) wyhash64_x_mt * 0xa3b195354a39b70d;
        uint64_t m1 = (tmp >> 64) ^tmp;
        tmp = (__uint128_t) m1 * 0x1b03738712fad5c9;
        uint64_t m2 = (tmp >> 64) ^tmp;
        return m2;
    }
};

#endif //SAND_CHUNKWORKER_H
