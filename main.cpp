//
// Created by tdolega on 14.04.2021.
//

#include "ParticleSystem.h"

int main() {
    sf::Clock clock;
    sf::RenderWindow window(sf::VideoMode(W * PIXEL_SIZE, H * PIXEL_SIZE), "sand");
    if(FPS_LIMIT) window.setFramerateLimit(FPS_LIMIT);
    sf::View view(sf::Vector2f((float)W/2, (float)H/2), sf::Vector2f(W, H));
    window.setView(view);
    sf::RenderTexture buffer;
    buffer.create(W, H);
    sf::Sprite bufferSprite(buffer.getTexture());

    ParticleSystem simulation;
    simulation.fillHalf();

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
//        simulation.update_ST(elapsed);
        simulation.update_MT(elapsed);

        ms += elapsed.asMicroseconds();
        lc++;
        if (ms >= 1000000) { // show fps every second
            int fps = 1000000 / ((float) ms / lc);
            cout << fps << " fps" << endl;
            ms = lc = 0;
        }

        buffer.clear();
        buffer.draw(renderer);
        buffer.display();

        window.clear();
        window.draw(bufferSprite);
        window.display();
    }

    return 0;
}
