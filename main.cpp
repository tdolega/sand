//
// Created by tdolega on 14.04.2021.
//

#include "ParticleSystem.h"

int main() {
    ParticleSystem simulation;
    sf::Clock clock;
    sf::RenderWindow window(sf::VideoMode(W * PIXEL_SIZE, H * PIXEL_SIZE), "sand");
//    window.setFramerateLimit(120);

    long lc = 0, ms = 0;
    while (window.isOpen()) {
        sf::Event event{};
        int wheelDelta = 0;
        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseWheelMoved)
                wheelDelta = event.mouseWheel.delta;
            else if (event.type == sf::Event::KeyPressed)
                simulation.handleKeyboard(event.key);

        auto mouse = sf::Mouse::getPosition(window);
        simulation.handleMouse(window.mapPixelToCoords(mouse), wheelDelta);

        auto elapsed = clock.restart();
        simulation.update(elapsed);

        ms += elapsed.asMicroseconds();
        lc++;
        if (ms >= 1000000) { // show fps every second
            int fps = 1000000 / ((float) ms / lc);
            cout << fps << " fps" << endl;
            ms = lc = 0;
        }

        window.clear();
        window.draw(renderer);
        window.display();
    }
    return 0;
}
