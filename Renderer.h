//
// Created by tdolega on 14.04.2021.
//

#ifndef SAND_RENDERER_H
#define SAND_RENDERER_H

#include "variables.h"

class Renderer : public sf::Drawable, public sf::Transformable {
    sf::VertexArray m_vertices;
    sf::VertexArray dupa = sf::VertexArray(sf::Lines, 5 );

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
//        states.transform *= getTransform();
        states.texture = nullptr;
        target.draw(m_vertices, states);
    }

public:
    Renderer() :
            m_vertices(sf::Points, W * H) {
                for (int y = 0; y < H; y++)
                    for (int x = 0; x < W; x++) {
                        auto &v = m_vertices[x + W * y];
                        v.position.x = x;
                        v.position.y = y;
                        v.color = sf::Color(0);
                    }
    }

    void updateVertex(const int x, const int y, const unsigned color) { updateVertex(x + W*y, color); }
    void updateVertex(const int idx, const unsigned color) {
        auto &v = m_vertices[idx];
        v.color = sf::Color(color);
    }
};

Renderer renderer;

#endif //SAND_RENDERER_H
