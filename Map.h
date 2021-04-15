//
// Created by tdolega on 14.04.2021.
//

#ifndef SAND_MAP_H
#define SAND_MAP_H

#include "Chunk.h"

class Map {

public:
    const int m_MXI = W / CHUNKSIZEX,
            m_MYI = H / CHUNKSIZEY,
            m_MI = m_MXI * m_MYI;
    Chunk **m_chunks;

    Map() :
            m_chunks(new Chunk *[m_MI]) {
        for (int y = 0; y < m_MYI; y++) {
            for (int x = 0; x < m_MXI; x++)
                m_chunks[x + m_MXI * y] = new Chunk(CHUNKSIZEX, CHUNKSIZEY, x * CHUNKSIZEX, y * CHUNKSIZEY);
        }
    }

    Particle &getParticle(const int x, const int y) const {
        return getChunk(x, y).getParticle(x, y);
    }

    Chunk &getChunk(const int x, const int y) const {
        return *m_chunks[getChunkIdx(x, y)];
    }

    int getChunkIdx(const int x, const int y) const {
        return (x / CHUNKSIZEX) + (y / CHUNKSIZEY) * m_MXI;
    }

    int getParticleGIdx(const int x, const int y) const {
        return x + W*y;
    }

    bool inBounds(const int x, const int y) const {
        return x >= 0 && x < W
            && y >= 0 && y < H;
    }
};

#endif //SAND_MAP_H
