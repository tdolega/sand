#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <thread>

using std::cout;
using std::endl;

//const unsigned W = 2400;
//const unsigned H = 1300;
//int brushsize = 80;

const unsigned W = 1200;
const unsigned H = 700;
int brushsize = 20;

//const unsigned W = 256;
//const unsigned H = 144;
//int brushsize = 3;

const float LIFETIME = 50000;

const int SAND1WATER0 = 1;

const int ITHREADS = 16;

uint64_t wyhash64_x;
uint64_t myrand() {
//    return 1;
//    return rand();
    wyhash64_x += 0x60bee2bee120fc15;
    __uint128_t tmp;
    tmp = (__uint128_t) wyhash64_x * 0xa3b195354a39b70d;
    uint64_t m1 = (tmp >> 64) ^ tmp;
    tmp = (__uint128_t)m1 * 0x1b03738712fad5c9;
    uint64_t m2 = (tmp >> 64) ^ tmp;
    return m2;
}

class Particle {
public:
//    sf::Vector2f velocity;
    sf::Vector2u position;
    sf::Time lifetime;

    Particle(unsigned x, unsigned y, sf::Time lifetimeAprox) {
        position.x = x;
        position.y = y;
//        float angle = ((myrand() % 20) + 80) * 3.14f / 180.f;
//        float speed = (myrand() % 50) + 50.f;
//        velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        auto lax = lifetimeAprox.asMilliseconds();
        lifetime = sf::milliseconds((myrand() % lax) * 0.25 + lax);
    }
};

class ParticleSystem : public sf::Drawable, public sf::Transformable {
    const unsigned m_w, m_h;
    Particle **m_particles;
    short *m_moved;
    sf::VertexArray m_vertices;
    const sf::Time m_lifetime = sf::seconds(LIFETIME);

public:
    explicit ParticleSystem(unsigned w, unsigned h) :
            m_w(w), m_h(h),
            m_particles(new Particle *[w * h]()),
            m_moved(new short[w * h]()),
            m_vertices(sf::Points, w * h) {
        for (unsigned y = 0; y < h; y++) {
            for (unsigned x = 0; x < w; x++) {
                auto &v = m_vertices[x + m_w * y];
                v.position.x = x;
                v.position.y = y;
                v.color.r = 227; // pink
                v.color.g = 36;
                v.color.b = 214;
                v.color.a = 0;
            }
        }
    }

    ~ParticleSystem() override {
        for (unsigned i = 0; i < m_w * m_h; i++)
            delete m_particles[i];
        delete[] m_particles;
        delete[] m_moved;
    }

    void handleMouse(sf::Vector2f position, int wheelDelta) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
            removeParticles(position);
        else if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            spawnNewParticles(position);

        int newBrushSize = brushsize + wheelDelta * 4;
        if (wheelDelta && newBrushSize > 0)
            brushsize = newBrushSize;
    }

    void update(sf::Time elapsed) {
        clearMMoved();

        updateRange(elapsed, 0, m_h);

//        for (int i = m_h - 1; i >= 0; --i) {
//        for(int j = 0; j < m_w; ++j){

//        for(int i = 0; i < m_h; ++i){
//            for (int j = m_w - 1; j >= 0; --j) {
//                updateParticle(j + m_w * i, elapsed);
//            }
//        }
    }

    void update_v2(sf::Time elapsed) {
        clearMMoved();

        for(int j = 0; j < m_w; ++j){
        for(int i = 0; i < m_h; ++i){
//            for (int j = m_w - 1; j >= 0; --j) {
//                for (int i = m_h - 1; i >= 0; --i) {
                updateParticle_v2(j + m_w * i, elapsed);
            }
        }
    }

    void updateFakeMultithreaded(sf::Time elapsed) {
        clearMMoved();

        std::thread t1(
                &ParticleSystem::updateRange, this, elapsed, 0, m_h
        );
        t1.join();
    }

    void updateMultithreaded(sf::Time elapsed) {
        clearMMoved();

        const int slice = m_h / ITHREADS;
        if (slice < 10) {
            cout << "ERROR: too many threads, slice = " << slice;
            exit(1);
        }
        std::vector<std::thread> threads;

        for (int ijob = 0; ijob < ITHREADS; ijob += 2)
            threads.push_back(std::thread(
                    &ParticleSystem::updateRange, this, elapsed, ijob * slice, (ijob + 1) * slice
            ));
        for (auto &thread : threads)
            thread.join();

        threads.clear();
        for (int ijob = 1; ijob < ITHREADS; ijob += 2)
            threads.emplace_back(
                    &ParticleSystem::updateRange, this, elapsed, ijob * slice, (ijob + 1) * slice
            );
        int leftovers = slice * ITHREADS;
        if (leftovers)
            threads.emplace_back(
                    &ParticleSystem::updateRange, this, elapsed, leftovers, m_h
            );
        for (auto &thread : threads)
            thread.join();
    }

private:
    void updateRange(sf::Time elapsed, int im, int iM) {
        for (int i = im; i < iM; ++i) {
            for (int j = m_w - 1; j >= 0; --j) {
                updateParticle(j + m_w * i, elapsed);
            }
        }
    }

    void clearMMoved() {
        std::fill(m_moved, m_moved + m_w * m_h, 0);
    }

    void spawnNewParticles(sf::Vector2f position) {
        unsigned idx;
        int fillPercentage = 1;
        for (int x = -brushsize; x <= brushsize; x++) {
            int circle = sqrt(pow(brushsize, 2) - pow(x, 2));
            for (int y = -circle; y <= circle; y++) {
                if (myrand() % 100 > fillPercentage) continue;
                const unsigned nx = position.x + x;
                const unsigned ny = position.y + y;
                if (xytoi(nx, ny, idx) && !m_particles[idx]) {
                    m_particles[idx] = new Particle(nx, ny, m_lifetime);
                    m_vertices[idx].color.a = 255;
                }
            }
        }
    }

    void removeParticles(sf::Vector2f position) {
        unsigned idx;
        for (int x = -brushsize; x <= brushsize; x++) {
            int circle = sqrt(pow(brushsize, 2) - pow(x, 2));
            for (int y = -circle; y <= circle; y++) {
                const unsigned nx = position.x + x;
                const unsigned ny = position.y + y;
                if (xytoi(nx, ny, idx) && m_particles[idx]) {
                    delete m_particles[idx];
                    m_particles[idx] = nullptr;
                    m_vertices[idx].color.a = 0;
                }
            }
        }
    }

    bool updateParticleTime(unsigned idx, Particle *p, sf::Time elapsed) {
        p->lifetime -= elapsed;
        if (p->lifetime <= sf::Time::Zero) {
            m_vertices[idx].color.a = 0;
            delete m_particles[idx];
            m_particles[idx] = nullptr;
            return false;
        }
//        float ratio = p->lifetime.asMilliseconds() / m_lifetime.asMilliseconds();
//        m_vertices[idx].color.a = static_cast<sf::Uint8>(ratio * 255);
        return true;
    }

    bool getNextPosition(unsigned &x, unsigned &y, unsigned &idx, bool moved) const {
//        int ny = y + 1;
//        if (xytoi(x, ny, idx) && !m_particles[idx]) {
//            y = ny;
//            return true;
//        }
        int ny;
        if(!moved)
            for (ny = y + 4 + myrand() % 4; ny > y; --ny) {
                if (xytoi(x, ny, idx) && !m_particles[idx]) {
                    y = ny;
                    return true;
                }
            }
        else if(xytoi(x, y + 1, idx) && !m_particles[idx])
            return false;

        ny = y + SAND1WATER0;

        int direction = (myrand() % 2) ? 1 : -1;
        int nx = x + direction;
        if (xytoi(nx, ny, idx) && !m_particles[idx]) {
            x = nx;
            y = ny;
            return true;
        }
        nx = x - direction;
        if (xytoi(nx, ny, idx) && !m_particles[idx]) {
            x = nx;
            y = ny;
            return true;
        }
        return false;
    }

    bool getNextPosition_v2(unsigned &x, unsigned &y, unsigned &idx) const {
        int ny;
        ny = y - 1;
        if (xytoi(x, ny, idx) && m_particles[idx] && !m_moved[idx]) {
            y = ny;
            return true;
        }
//        if(!moved)
//            for (ny = y - 4 - myrand() % 4; ny < y; ny++) {
//                if (xytoi(x, ny, idx) && m_particles[idx] && !m_moved[idx]) {
//                    y = ny;
//                    return true;
//                }
//            }
//        else if(xytoi(x, y + 1, idx) && !m_particles[idx])
//            return false;

        ny = y - SAND1WATER0;

        int direction = (myrand() % 2) ? 1 : -1;
        int nx = x + direction;
        if (xytoi(nx, ny, idx) && m_particles[idx] && !m_moved[idx] && m_particles[idx+m_w]) {
            x = nx;
            y = ny;
            return true;
        }
        nx = x - direction;
        if (xytoi(nx, ny, idx) && m_particles[idx] && !m_moved[idx] && m_particles[idx+m_w]) {
            x = nx;
            y = ny;
            return true;
        }
        return false;
    }

    void updateParticle(unsigned idx, sf::Time elapsed) {
        auto &p = m_particles[idx];
        if (!p) return;
        if (m_moved[idx] > 0) return;

//        if (!updateParticleTime(idx, p, elapsed)) return;

        auto nx = idx % m_w;
        auto ny = idx / m_w;
        auto nidx = idx;
        if (getNextPosition(nx, ny, nidx, m_moved[idx])) {
            p->position.x = nx;
            p->position.y = ny;
            m_vertices[idx].color.a = 0;
            m_vertices[nidx].color.a = 255;
            std::swap(m_particles[idx], m_particles[nidx]);
            m_moved[nidx]++;
        }
    }

    void updateParticle_v2(unsigned idx, sf::Time elapsed) {
        auto &p = m_particles[idx];
        if (p) return;
//        if (m_moved[idx] > 0) return;

//        if (!updateParticleTime(idx, p, elapsed)) return;

        auto nx = idx % m_w;
        auto ny = idx / m_w;
        auto nidx = idx;
        if (getNextPosition_v2(nx, ny, nidx)) {
            std::swap(m_particles[idx], m_particles[nidx]);
            p->position.x = nx;
            p->position.y = ny;
            m_vertices[nidx].color.a = 0;
            m_vertices[idx].color.a = 255;
            m_moved[idx]++;
        }
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
//        states.transform *= getTransform();
        states.texture = nullptr;
        target.draw(m_vertices, states);
    }

    inline unsigned xytoi(int x, int y, unsigned &idx) const {
        if (x < 0 || x >= m_w || y < 0 || y >= m_h)
            return false;
        idx = x + m_w * y;
        return true;
    }
};


int main() {
    sf::RenderWindow window(sf::VideoMode(W, H), "sand");
//    window.setFramerateLimit(120);

    sf::Clock clock;

    ParticleSystem particles(W, H);

    // run the main loop
    long lc = 0;
    long ms = 0;
    while (window.isOpen()) {
        // handle events
        sf::Event event{};
        int wheelDelta = 0;
        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseWheelMoved)
                wheelDelta = event.mouseWheel.delta;

        sf::Vector2i mouse = sf::Mouse::getPosition(window);
        particles.handleMouse(window.mapPixelToCoords(mouse), wheelDelta);

        sf::Time elapsed = clock.restart();
//        particles.update(elapsed);
//        particles.updateFakeMultithreaded(elapsed);
//        particles.updateMultithreaded(elapsed);
        particles.update_v2(elapsed);

        ms += elapsed.asMicroseconds();
        lc++;
        if (ms >= 1000000) { // show fps every second
            int fps = 1000000 / ((float) ms / lc);
            cout << fps << " fps" << endl;
            ms = 0;
            lc = 0;
        }

        window.clear();
        window.draw(particles);
        window.display();
    }
    return 0;
}