//
// Created by tdolega on 14.04.2021.
//

#ifndef SAND_CHUNK_H
#define SAND_CHUNK_H

#include "Particle.h"

class Chunk {
public:
    Particle *m_particles;
    std::vector<std::pair<int, int>> m_changes;
    std::vector<std::pair<int, int>> m_changes_mtx;

    const int m_CW, m_CH, m_CX, m_CY, m_CI = m_CW * m_CH;
    int m_ym = 0, m_yM = m_CH, m_xm = 0, m_xM = m_CW,
            m_tym = m_ym, m_tyM = m_yM, m_txm = m_xm, m_txM = m_xM;


    Chunk(const int CW, const int CH, const int CX, const int CY)
            : m_CW(CW), m_CH(CH), m_CX(CX), m_CY(CY),
              m_particles(new Particle[CW * CH]()) {}

    ~Chunk() {
        delete[] m_particles;
    }

    bool inBounds(const int x, const int y) const {
        return x >= m_CX && x < m_CX + m_CW
               && y >= m_CY && y < m_CY + m_CH;
    }

    Particle *getParticle(const int x, const int y) const {
        return getParticle(getParticleIdx(x, y));
    }

    Particle *getParticle(const int idx) const {
        return &m_particles[idx];
    }

    int getParticleIdx(const int x, const int y) const {
        return (x - m_CX) + (y - m_CY) * m_CW;
    }

    void getXY(const int idx, int &x, int &y) {
        x = idx % m_CW + m_CX;
        y = idx / m_CW + m_CY;
    }

//    void setParticle(const int x, const int y, Particle *& p) {
//        m_particles[getParticleIdx(x, y)] = std::move(p);
//    }

    void scheduleMove(const int nidx, const int idx) {
        m_changes.emplace_back(nidx, idx);
    }

    void commitMoves() {
//        std::sort(std::execution::par_unseq, m_changes.begin(), m_changes.end(),
        std::sort(m_changes.begin(), m_changes.end(),
                  [](auto &a, auto &b) { return a.first < b.first; }
        );

        m_changes.emplace_back(-1, -1); // catch final move

        int iprev = 0;
        for (int i = 0; i < m_changes.size() - 1; i++) {
            if (m_changes[i + 1].first != m_changes[i].first) {
                int rand = iprev + myrand() % (i - iprev + 1);
                auto[dst, src] = m_changes[rand];
                std::swap(m_particles[dst], m_particles[src]);

                int x, y;
                getXY(dst, x, y);
                renderer.updateVertex(x, y, m_particles[dst].type);;
                getXY(src, x, y);
                renderer.updateVertex(x, y, m_particles[src].type);

                iprev = i + 1;
            }
        }
        m_changes.clear();
    }
};

#endif //SAND_CHUNK_H
