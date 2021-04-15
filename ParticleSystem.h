//
// Created by tdolega on 14.04.2021.
//

#ifndef SAND_PARTICLESYSTEM_H
#define SAND_PARTICLESYSTEM_H

#include "ChunkWorker.h"

class ParticleSystem {
    ChunkWorker **m_chunkWorkers;
    Map m_map;
    types m_spawnT = T_SAND;

public:
    ParticleSystem() {
        m_chunkWorkers = new ChunkWorker *[m_map.m_MI];
        for (int i = 0; i < m_map.m_MI; i++)
            m_chunkWorkers[i] = new ChunkWorker(m_map, *m_map.m_chunks[i]);

    }

    void update(sf::Time &elapsed) {
//#pragma omp parallel for
        for (int i = 0; i < m_map.m_MI; i++)
            m_chunkWorkers[i]->update(elapsed);

//#pragma omp parallel for
        for (int i = 0; i < m_map.m_MI; i++)
            m_chunkWorkers[i]->commit();
    }

    void handleMouse(const sf::Vector2f &position, const int wheelDelta) {
        int newBrushSize = brushSize + wheelDelta * 4;
        if (wheelDelta && newBrushSize > 0)
            brushSize = newBrushSize;

        if (!m_map.inBounds((int) position.x / PIXEL_SIZE, (int) position.y / PIXEL_SIZE)) return;

        if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
            removeParticles(position);
        else if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            spawnParticles(position);
    }

    void handleKeyboard(const sf::Event::KeyEvent keycode) {
        if (keycode.code == sf::Keyboard::Numpad7)
            m_spawnT = T_SAND;
        else if (keycode.code == sf::Keyboard::Numpad8)
            m_spawnT = T_WATER;
        else if (keycode.code == sf::Keyboard::Numpad9)
            m_spawnT = T_STONE;
        else if (keycode.code == sf::Keyboard::Numpad4)
            m_spawnT = T_SANDPINK;
    }

    void fillHalf() { // debug
        for(int y = 0; y < H/2; y++)
            for(int x = 0; x < W; x++){
                auto &c = m_map.getChunk(x, y);
                c.setParticle(x, y, Particle(myrand()%2 ? T_SAND : T_WATER));
            }
    }

private:
    void spawnParticles(const sf::Vector2f &position) {
        for (int x = -brushSize; x <= brushSize; x++) {
            const int circle = (int) sqrt(pow(brushSize, 2) - pow(x, 2));
            for (int y = -circle; y <= circle; y++) {
                if (m_spawnT != T_STONE
                    && myrand() % 100 > FILL_PERC)
                    continue;
                const int nx = (int) position.x / PIXEL_SIZE + x;
                const int ny = (int) position.y / PIXEL_SIZE + y;
                if (!m_map.inBounds(nx, ny)) continue;
                auto &c = m_map.getChunk(nx, ny);
                auto &p = c.getParticle(nx, ny);
                if (p.type != T_EMPTY) continue;
                c.setParticle(nx, ny, Particle(m_spawnT));
            }
        }
    }

    void removeParticles(const sf::Vector2f &position) {
        for (int x = -brushSize; x <= brushSize; x++) {
            int circle = (int) sqrt(pow(brushSize, 2) - pow(x, 2));
            for (int y = -circle; y <= circle; y++) {
                const int nx = (int) position.x / PIXEL_SIZE + x;
                const int ny = (int) position.y / PIXEL_SIZE + y;
                if (!m_map.inBounds(nx, ny)) continue;
                auto &c = m_map.getChunk(nx, ny);
                auto &p = c.getParticle(nx, ny);
                if (p.type == T_EMPTY) continue;
                c.setParticle(nx, ny, Particle());
            }
        }
    }

};

#endif //SAND_PARTICLESYSTEM_H
