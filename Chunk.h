//
// Created by tdolega on 14.04.2021.
//

#ifndef SAND_CHUNK_H
#define SAND_CHUNK_H

#include "Particle.h"

class Chunk {
public:
    Particle *m_particles;
    std::vector<std::tuple<Particle &, Particle &, int, int>> m_swaps;
    std::mutex m_add;

    const int m_CW, m_CH, m_CX, m_CY, m_CI;
    int m_ym = 0, m_yM = m_CH, m_xm = 0, m_xM = m_CW,
            m_tym = m_ym, m_tyM = m_yM, m_txm = m_xm, m_txM = m_xM;


    explicit Chunk(const int CW, const int CH, const int CX, const int CY)
            : m_CW(CW), m_CH(CH), m_CX(CX), m_CY(CY), m_CI(CW*CH),
              m_particles(new Particle[CW * CH]) {}

    ~Chunk() {
        delete[] m_particles;
    }

    bool inBounds(const int x, const int y) const {
        return x >= m_CX && x < m_CX + m_CW
               && y >= m_CY && y < m_CY + m_CH;
    }

    Particle &getParticle(const int x, const int y) {
        return getParticle(getParticleIdx(x, y));
    }
    Particle &getParticle(const int idx) const {
        return m_particles[idx];
    }

    int getParticleIdx(const int x, const int y) const {
        return (x - m_CX) + (y - m_CY) * m_CW;
    }

    std::pair<int, int> getXY(const int idx) {
        const int x = idx % m_CW + m_CX;
        const int y = idx / m_CW + m_CY;
        return std::pair(x, y);
    }

    void setParticle(const int x, const int y, const Particle& p) {
        getParticle(x, y) = p;
        renderer.updateVertex(x, y, p.type);
    }

    void scheduleMove(Particle &np, Particle &p, const int nidx, const int idx) {
        m_add.lock();
        m_swaps.emplace_back(np, p, nidx, idx);
        m_add.unlock();
    }
};

#endif //SAND_CHUNK_H
