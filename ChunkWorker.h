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

    Particle* getNextPosition(const Particle &p, int &x, int &y) const {
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
                        && np->type != p.type));
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

        int direction = p.velocity.x
                        ? (p.velocity.x & -1)
                        : (myrand() % 2) ? 1 : -1;
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
            c.scheduleMove(*np, p, ngidx, gidx);
        } else {
            p.velocity.x = 0;
            p.velocity.y = 0;
        }
    }

    void commit() {
        auto &swaps = m_chunk.m_swaps;
//        std::sort(std::execution::par_unseq, swaps.begin(), swaps.end(),
        std::sort(swaps.begin(), swaps.end(),
                  [](auto &a, auto &b) { return std::get<2>(a) < std::get<2>(b); }
        );

        Particle &dupa = std::get<1>(swaps[0]);

        Particle end;
        swaps.emplace_back(end, end, -1, -1); // catch final move

        int iprev = 0;
        for (int i = 0; i < swaps.size() - 1; i++) {
            if (std::get<2>(swaps[i + 1]) != std::get<2>(swaps[i])) {
                int rand = iprev + myrand() % (i - iprev + 1);
                auto &[dp, sp, didx, sidx] = swaps[rand];
//                std::swap(dp, sp);
                Particle &dp2 = m_map.getParticle(didx%W, didx/W);
                Particle &sp2 = m_map.getParticle(sidx%W, sidx/W);
                std::swap(dp2, sp2);
                renderer.updateVertex(didx, dp2.type);
                renderer.updateVertex(sidx, sp2.type);
                iprev = i + 1;
            }
        }
        swaps.clear();

        for(int i = 0; i < m_chunk.m_CI; i++) { // todo delete
            auto [x, y] = m_chunk.getXY(i);
            renderer.updateVertex(x, y, m_chunk.getParticle(i).type);
        }
    }
};

#endif //SAND_CHUNKWORKER_H
