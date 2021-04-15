//
// Created by tdolega on 14.04.2021.
//

#ifndef SAND_RENDERER_H
#define SAND_RENDERER_H

#include "variables.h"

class Renderer : public sf::Drawable, public sf::Transformable {

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
//        states.transform *= getTransform();
        states.texture = nullptr;
        target.draw(m_vertices, states);
    }

public:
    sf::VertexArray m_vertices;
    Renderer() :
            m_vertices(sf::Points, W * H * PIXEL_SIZE * PIXEL_SIZE) {
        for (int py = 0; py < PIXEL_SIZE; py++)
            for (int px = 0; px < PIXEL_SIZE; px++)
                for (int y = 0; y < H; y++)
                    for (int x = 0; x < W; x++) {
                        auto &v = m_vertices[(x + W * y) + W * H * (px + py * PIXEL_SIZE)];
                        v.position.x = x * PIXEL_SIZE + px;
                        v.position.y = y * PIXEL_SIZE + py;
                        v.color = sf::Color(0);
                    }
    }

    void updateVertex(const int x, const int y, const unsigned color) { updateVertex(x + W*y, color); }
    void updateVertex(const int idx, const unsigned color) {
        for (int pixel = 0; pixel < PIXEL_SIZE * PIXEL_SIZE; pixel++) {
            auto &v = m_vertices[idx + W*H*pixel];
            v.color = sf::Color(color);
        }
    }
};

Renderer renderer;

#endif //SAND_RENDERER_H
