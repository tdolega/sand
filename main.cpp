#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <thread>

using std::cout;
using std::endl;

//const int W = 2400;
//const int H = 1300;
//int brushSize = 80;

//const int W = 1200;
//const int H = 700;
//int brushSize = 20;

const int W = 256;
const int H = 144;
int brushSize = 3;

const int PIXEL_SIZE = 4;

//const float LIFETIME_MS = 50000;

const int ITHREADS = 16;

const unsigned SAND_C = 0xc2b280ff;
const unsigned WATER_C = 0x2389daff;
const unsigned STONE_C = 0x888c8dff;
const unsigned PINK_C = 0xe324d6ff;
const unsigned EMPTY_C = 0;

uint64_t wyhash64_x;
uint64_t myrand() {
//    return 1;
//    return rand();
    wyhash64_x += 0x60bee2bee120fc15;
    __uint128_t tmp;
    tmp = (__uint128_t) wyhash64_x * 0xa3b195354a39b70d;
    uint64_t m1 = (tmp >> 64) ^tmp;
    tmp = (__uint128_t) m1 * 0x1b03738712fad5c9;
    uint64_t m2 = (tmp >> 64) ^tmp;
    return m2;
}

//const sf::Time LIFETIME = sf::seconds(LIFETIME_MS);

const unsigned SAND   = 0b000001;
const unsigned WATER  = 0b000010;
const unsigned STONE  = 0b000100;
const unsigned EMPTY  = 0;

class Particle {
public:
    int type = 0;
    sf::Time lifetime;
    bool updated = false;
    sf::Vector2f velocity {0, 0};

    Particle() {
//        float angle = ((myrand() % 20) + 80) * 3.14f / 180.f;
//        float speed = (myrand() % 50) + 50.f;
//        velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
    }

    void setLifetime(sf::Time lifetimeAprox) {
        auto lax = lifetimeAprox.asMilliseconds();
        lifetime = sf::milliseconds((myrand() % lax) * 0.25 + lax);
    }
};

class ParticleSystem : public sf::Drawable, public sf::Transformable {
    Particle *m_particles;
    sf::VertexArray m_vertices;
    std::vector<std::pair<int, int>> m_changes;
    int m_spawnType = SAND;

public:
    explicit ParticleSystem() :
            m_particles(new Particle[W * H]()),
            m_vertices(sf::Points, W * H * PIXEL_SIZE*PIXEL_SIZE) {
        for(int py = 0; py < PIXEL_SIZE; py++)
        for(int px = 0; px < PIXEL_SIZE; px++)
            for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++) {
                auto &v = m_vertices[(x + W * y) + W*H*(px+py*PIXEL_SIZE)];
                v.position.x = x * PIXEL_SIZE + px;
                v.position.y = y * PIXEL_SIZE + py;
                v.color = sf::Color(EMPTY_C);
            }
    }

    ~ParticleSystem() override {
        delete[] m_particles;
    }

    void handleMouse(sf::Vector2f position, int wheelDelta) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
            removeParticles(position);
        else if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            spawnParticles(position);

        int newBrushSize = brushSize + wheelDelta * 4;
        if (wheelDelta && newBrushSize > 0)
            brushSize = newBrushSize;
    }

    void handleKeyboard(sf::Event::KeyEvent keycode) {
        if     (keycode.code == sf::Keyboard::Numpad0)
            m_spawnType = SAND;
        else if(keycode.code == sf::Keyboard::Numpad1)
            m_spawnType = WATER;
        else if(keycode.code == sf::Keyboard::Numpad2)
            m_spawnType = STONE;
    }

    void update(sf::Time elapsed) {
        clearMoved();

        updateRange(elapsed, 0, H);

        commitMoves();
//        updateVertices();

//        for (int i = m_h - 1; i >= 0; --i) {
//        for(int j = 0; j < m_w; ++j){

//        for(int i = 0; i < m_h; ++i){
//            for (int j = m_w - 1; j >= 0; --j) {
//                updateParticle(j + m_w * i, elapsed);
//            }
//        }
    }

    void update_singlethread(sf::Time elapsed) {
        clearMoved();

        std::thread t1(
                &ParticleSystem::updateRange, this, elapsed, 0, H
        );
        t1.join();

        commitMoves();
        updateVertices();

    }

    void update_multithread(sf::Time elapsed) {
        clearMoved();

        const int slice = H / ITHREADS;
        if (slice < 10) { // irrevelant as for today
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
                    &ParticleSystem::updateRange, this, elapsed, leftovers, H
            );
        for (auto &thread : threads)
            thread.join();


        commitMoves();
        updateVertices();
    }

private:
    void updateRange(const sf::Time elapsed, const int ym, const int yM) {
        for (int y = ym; y < yM; ++y) {
            for (int x = 0; x < W; ++x) {
                updateParticle(elapsed, x, y);
            }
        }
    }

    void clearMoved() {
        for (int i = 0; i < W * H; i++)
            m_particles[i].updated = false;
    }

    void spawnParticles(sf::Vector2f &position) {
        Particle *p;
        int fillPercentage = 1;
        for (int x = -brushSize; x <= brushSize; x++) {
            int circle = sqrt(pow(brushSize, 2) - pow(x, 2));
            for (int y = -circle; y <= circle; y++) {
                if (m_spawnType != STONE && myrand() % 100 > fillPercentage) continue;
                const int nx = position.x / PIXEL_SIZE + x;
                const int ny = position.y / PIXEL_SIZE + y;
                if (xytop(nx, ny, p) && !p->type) {
                    p->type = m_spawnType;
//                    Particle p2;
                    updateVertex(nx+W*ny);
                }
            }
        }
    }

    void removeParticles(sf::Vector2f &position) {
        Particle *p;
        for (int x = -brushSize; x <= brushSize; x++) {
            int circle = sqrt(pow(brushSize, 2) - pow(x, 2));
            for (int y = -circle; y <= circle; y++) {
                const int nx = position.x / PIXEL_SIZE + x;
                const int ny = position.y / PIXEL_SIZE + y;
                if (xytop(nx, ny, p) && p->type) {
                    p->type = EMPTY;
                    updateVertex(nx+W*ny);
                }
            }
        }
    }

//    bool updateParticleTime(int idx, Particle *p, sf::Time elapsed) {
//        p->lifetime -= elapsed;
//        if (p->lifetime <= sf::Time::Zero) {
//            m_vertices[idx].color.a = 0;
//            delete m_particles[idx];
//            m_particles[idx] = nullptr;
//            return false;
//        }
////        float ratio = p->lifetime.asMilliseconds() / m_lifetime.asMilliseconds();
////        m_vertices[idx].color.a = static_cast<sf::Uint8>(ratio * 255);
//        return true;
//    }

    bool getNextPosition(int &x, int &y, Particle *&p, Particle *&np) const {
        int swapWith = EMPTY; // (tnp->type & swapWith)
        Particle *tnp = nullptr;
        int ty, tx, im, iM;
//        if(p->velocity.x && p->velocity.y) {
//            iM = p->velocity.y; // +1 +2 maybe even
//            for (int i = 1; i <= iM; ++i) {
//                tx = x + i * ((int)p->velocity.x & -1);
//                ty = y + i * ((int)p->velocity.y & -1);
//                if (xytop(tx, ty, tnp) && tnp->type == swapWith) {
//                    np = tnp;
//                    x = tx;
//                    y = ty;
//                } else break;
//            }
//            if (np) return true;
//        } else
//        if (p->velocity.x) { // TODO check without else
//            im = x;
//            iM = x + ((int)p->velocity.x & -1);
//            if(im > iM)
//                std::swap(im, iM);
//            for (int tx = im; tx <= iM; ++tx) {
//                if (xytop(tx, y, tnp) && tnp->type == swapWith) {
//                    np = tnp;
//                    x = tx;
//                } else break;
//            }
//            if (np) return true;
//        } else
        if (p->velocity.y) {
            im = y;
            iM = y + ((int)p->velocity.y & -1);
            if(im > iM)
                std::swap(im, iM);
            for (int ty = im; ty <= iM; ++ty) {
                if (xytop(x, ty, tnp) && tnp->type == swapWith) {
                    np = tnp;
                    y = ty;
                } else break;
            }
            if (np) return true;
        }

        ty = y + 1;
        if (xytop(x, ty, tnp) && tnp->type == swapWith) {
            np = tnp;
            y = ty;
            return true;
        }
        if (p->type == WATER) {
            int direction = p->velocity.x
                            ? ((int)p->velocity.x & -1)
                            : (myrand() % 2) ? 1 : -1;
            tx = x + direction;
            if (xytop(tx, y, tnp) && tnp->type == swapWith) {
                np = tnp;
                x = tx;
                return true;
            }
            tx = x - direction;
            if (xytop(tx, y, tnp) && tnp->type == swapWith) {
                np = tnp;
                x = tx;
                return true;
            }
        }
        int direction = p->velocity.x
                ? ((int)p->velocity.x & -1)
                : (myrand() % 2) ? 1 : -1;
        tx = x + direction;
        if (xytop(tx, ty, tnp) && tnp->type == swapWith) {
            np = tnp;
            x = tx;
            y = ty;
            return true;
        }
        tx = x - direction;
        if (xytop(tx, ty, tnp) && tnp->type == swapWith) {
            np = tnp;
            x = tx;
            y = ty;
            return true;
        }
        return false;
    }

    void updateParticle(sf::Time elapsed, int x, int y) {
        auto idx = x + W * y;
        auto *p = &m_particles[idx];
        if (!p->type || p->type == STONE) return;
        if (p->updated) return;

//        if (!updateParticleTime(idx, p, elapsed)) return;

        auto nx = x, ny = y, nidx = idx;
        Particle *np = nullptr;
        if (getNextPosition(nx, ny, p, np)) {
            nidx = nx + W * ny;
//            p->velocity.x = (nx - x);
            p->velocity.x = (nx - x) & -1;
            p->velocity.y = pow(ny - y + 1, 2);
            m_changes.emplace_back(nidx, idx);
        }
        else {
            p->velocity.y = 0;
            p->velocity.x = 0;
        }
    }

    void commitMoves() {
        std::sort(m_changes.begin(), m_changes.end(),
        [](auto& a, auto& b) { return a.first < b.first; }
        );
        m_changes.emplace_back(-1, -1); // catch final move

        int iprev = 0;
        for (size_t i = 0; i < m_changes.size() - 1; i++) {
            // clear moved
            if (m_changes[i + 1].first != m_changes[i].first) {
                // update vertex
                int rand = iprev + myrand() % (i - iprev + 1);
                int dst = m_changes[rand].first;
                int src = m_changes[rand].second;
                std::swap(m_particles[src], m_particles[dst]);
                iprev = i + 1;

                updateVertex(dst);
                updateVertex(src);
            }
        }
        m_changes.clear();
    }

    void updateVertices() {
        for (int i = 0; i < W * H; i++) {
            updateVertex(i);
        }
    }

    void updateVertex(int index) {
        for(int pixel = 0; pixel < PIXEL_SIZE*PIXEL_SIZE; pixel++) {
            auto &p = m_particles[index];
            auto &v = m_vertices[index + W * H * pixel];
            switch (p.type) {
                case SAND:
                    v.color = sf::Color(SAND_C);
                    continue;
                case WATER:
                    v.color = sf::Color(WATER_C);
                    continue;
                case STONE:
                    v.color = sf::Color(STONE_C);
                    continue;
                default:
                    v.color = sf::Color(EMPTY_C);
                    continue;
            }
        }
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
//        states.transform *= getTransform();
        states.texture = nullptr;
        target.draw(m_vertices, states);
    }

    inline bool xytop(int x, int y, Particle *&p) const {
        if (x < 0 || x >= W || y < 0 || y >= H)
            return false;
        p = &m_particles[x + W * y];
        return true;
    }
};


int main() {
    sf::RenderWindow window(sf::VideoMode(W * PIXEL_SIZE, H * PIXEL_SIZE), "sand");
//    window.setFramerateLimit(120);

    sf::Clock clock;

    ParticleSystem particles;

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
            else if (event.type == sf::Event::KeyPressed)
                particles.handleKeyboard(event.key);

        sf::Vector2i mouse = sf::Mouse::getPosition(window);
        particles.handleMouse(window.mapPixelToCoords(mouse), wheelDelta);

        sf::Time elapsed = clock.restart();
        particles.update(elapsed);
//        particles.update_singlethread(elapsed);
//        particles.update_multithread(elapsed);

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
