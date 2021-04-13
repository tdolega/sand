#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <execution>
#include <algorithm>
#include <parallel/algorithm>
#include <thread>
//#include <mutex>

using std::cout;
using std::endl;

//const int W = 2400;
//const int H = 1300;
//int brushSize = 80;

//const int W = 1200;
//const int H = 700;
//int brushSize = 20;
//const int PIXEL_SIZE = 1;

const int W = 256;
const int H = 144;
int brushSize = 3;
const int PIXEL_SIZE = 4;


//const float LIFETIME_MS = 50000;
const int FILL_PERC = 1;

const int ITHREADS = 4;

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

enum types { // color and id
    T_EMPTY     = 0,
    T_SAND      = 0xc2b280ff,
    T_WATER     = 0x2389daff,
    T_STONE     = 0x888c8dff,
    T_LAVA,
    T_FIRE,
    T_SMOKE,
    T_SANDPINK  = 0xe324d6ff,
};

enum moves {
    MOVES_NONE  = 0b000000,
    MOVES_U     = 0b000001,
    MOVES_LR    = 0b000010,
    MOVES_D     = 0b000100,
    MOVES_LRD   = 0b001000,
    MOVES_LRU   = 0b010000,
    MOVES_FORCE = 0b100000,
};

struct Particle {
    types type = T_EMPTY;
    sf::Time lifetime;
    sf::Vector2i velocity {0, 0};
    int hp = 100;

    void setLifetime(sf::Time lifetimeAprox) {
        auto lax = lifetimeAprox.asMilliseconds();
        lifetime = sf::milliseconds((myrand() % lax) * 0.25 + lax);
    }

    [[nodiscard]] unsigned moveMask() const {
        switch(type) {
            case T_SAND:
            case T_SANDPINK:
                return MOVES_D | MOVES_LRD | MOVES_FORCE;
            case T_WATER:
                return MOVES_D | MOVES_LRD | MOVES_LR;
            case T_SMOKE:
                return MOVES_U | MOVES_LRU | MOVES_LR;
            default:
                return MOVES_NONE;
        }
    }
};

class ParticleSystem : public sf::Drawable, public sf::Transformable {
    Particle *m_particles;
    std::vector<std::pair<int, int>> m_changes;
    sf::VertexArray m_vertices;
    types m_spawnT = T_SAND;

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
                v.color = sf::Color(T_EMPTY);
            }
    }

    ~ParticleSystem() override {
        delete[] m_particles;
    }

    void handleMouse(const sf::Vector2f &position, const int wheelDelta) {
        int newBrushSize = brushSize + wheelDelta * 4;
        if (wheelDelta && newBrushSize > 0)
            brushSize = newBrushSize;

        if(!inBounds((int)position.x/PIXEL_SIZE, (int)position.y/PIXEL_SIZE)) return;

        if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
            removeParticles(position);
        else if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            spawnParticles(position);
    }

    void handleKeyboard(const sf::Event::KeyEvent keycode) {
        if     (keycode.code == sf::Keyboard::Numpad7)
            m_spawnT = T_SAND;
        else if(keycode.code == sf::Keyboard::Numpad8)
            m_spawnT = T_WATER;
        else if(keycode.code == sf::Keyboard::Numpad9)
            m_spawnT = T_STONE;
        else if(keycode.code == sf::Keyboard::Numpad4)
            m_spawnT = T_SANDPINK;
    }

    void update(const sf::Time &elapsed) {
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x)
                updateParticle(elapsed, x, y);
        commitMoves();
    }

    void fillHalf() { // debug
        for(int y = 0; y < H/2; y++)
            for(int x = 0; x < W; x++) {
                m_particles[x + W*y].type = T_SAND;
                updateVertex(x+W*y);
            }
    }

private:
    void spawnParticles(const sf::Vector2f &position) {
        Particle *p;
        for (int x = -brushSize; x <= brushSize; x++) {
            const int circle = (int)sqrt(pow(brushSize, 2) - pow(x, 2));
            for (int y = -circle; y <= circle; y++) {
                if (m_spawnT != T_STONE
                    && myrand() % 100 > FILL_PERC) continue;
                const int nx = position.x / PIXEL_SIZE + x;
                const int ny = position.y / PIXEL_SIZE + y;
                if (xytop(nx, ny, p) && !p->type) {
                    p->type = m_spawnT;
                    updateVertex(nx+W*ny);
                }
            }
        }
    }

    void removeParticles(const sf::Vector2f &position) {
        Particle *p;
        for (int x = -brushSize; x <= brushSize; x++) {
            int circle = (int)sqrt(pow(brushSize, 2) - pow(x, 2));
            for (int y = -circle; y <= circle; y++) {
                const int nx = position.x / PIXEL_SIZE + x;
                const int ny = position.y / PIXEL_SIZE + y;
                if (xytop(nx, ny, p) && p->type) {
                    p->type = T_EMPTY;
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
        Particle *tnp = nullptr;
        int ty, tx, im, iM;
        auto mm = p->moveMask();

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
//        if (p->velocity.x) {
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

        auto canSwap = [mm, &p](Particle *& tnp) {
            return (tnp->type == T_EMPTY
                    || ((mm & MOVES_FORCE)
                        && tnp->type != T_STONE
                        && tnp->type != p->type));
        };

//        if (p->velocity.y) { // TODO change to Bresenham algorithm
//            im = y;
//            iM = y + (p->velocity.y & -1);
//            if(im > iM)
//                std::swap(im, iM);
//            for (ty = im; ty <= iM; ++ty) {
//                if (xytop(x, ty, tnp) && canSwap(tnp)) {
//                    np = tnp;
//                    y = ty;
//                } else break;
//            }
//            if (np) return true;
//        }

        if(mm & MOVES_D) {
            ty = y + 1;
            if (xytop(x, ty, tnp) && canSwap(tnp)) {
                np = tnp;
                y = ty;
                return true;
            }
        }
        if(mm & MOVES_LR) {
            int direction = p->velocity.x
                            ? (p->velocity.x & -1)
                            : (myrand() % 2) ? 1 : -1;
            tx = x + direction;
            if (xytop(tx, y, tnp) && canSwap(tnp)) {
                np = tnp;
                x = tx;
                return true;
            }
            tx = x - direction;
            if (xytop(tx, y, tnp) && canSwap(tnp)) {
                np = tnp;
                x = tx;
                return true;
            }
        }
        if(mm & MOVES_LRD) {
            int direction = p->velocity.x
                            ? (p->velocity.x & -1)
                            : (myrand() % 2) ? 1 : -1;
            tx = x + direction;
            if (xytop(tx, ty, tnp) && canSwap(tnp)) {
                np = tnp;
                x = tx;
                y = ty;
                return true;
            }
            tx = x - direction;
            if (xytop(tx, ty, tnp) && canSwap(tnp)) {
                np = tnp;
                x = tx;
                y = ty;
                return true;
            }
        }
        return false;
    }

    void updateParticle(const sf::Time &elapsed, int x, int y) {
        auto idx = x+W*y;
        auto *p = &m_particles[idx];
        if (p->type == T_EMPTY || p->type == T_STONE) return;

//        if (!updateParticleTime(idx, p, elapsed)) return;

        auto nx = x, ny = y, nidx = idx;
        Particle *np = nullptr;
        if (getNextPosition(nx, ny, p, np)) {
//            p->velocity.x = (nx - x);
            p->velocity.x = (nx - x) & -1;
            p->velocity.y = (int)pow(ny - y + 1, 2);
            nidx = nx+W*ny;
            m_changes.emplace_back(nidx, idx);
        } else {
            p->velocity.x = 0;
            p->velocity.y = 0;
        }
    }

    void commitMoves() {
        std::sort(std::execution::par_unseq, m_changes.begin(), m_changes.end(),
        [](auto& a, auto& b) { return a.first < b.first; }
        );

//        __gnu_parallel::sort(m_changes.begin(), m_changes.end(),
//        [](auto& a, auto& b) { return a.first < b.first; }
//        );

        m_changes.emplace_back(-1, -1); // catch final move

        int iprev = 0;
        for (int i = 0; i < m_changes.size() - 1; i++) {
            if (m_changes[i + 1].first != m_changes[i].first) {
                int rand = iprev + myrand() % (i - iprev + 1);
                int dst = m_changes[rand].first;
                int src = m_changes[rand].second;
                std::swap(m_particles[dst], m_particles[src]);
                updateVertex(dst);
                updateVertex(src);
                iprev = i + 1;
            }
        }
        m_changes.clear();
    }

    inline void updateVertex(const int index) {
        for(int pixel = 0; pixel < PIXEL_SIZE*PIXEL_SIZE; pixel++) {
            auto &p = m_particles[index];
            auto &v = m_vertices[index + W*H*pixel];
            v.color = sf::Color(p.type);
        }
    }

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
//        states.transform *= getTransform();
        states.texture = nullptr;
        target.draw(m_vertices, states);
    }

    static inline bool inBounds(const int x, const int y) {
        return x >= 0 && x < W && y >= 0 && y < H;
    }

    inline bool xytop(const int x, const int y, Particle *&p) const {
        if (!inBounds(x, y)) return false;
        p = &m_particles[x+W*y];
        return true;
    }
};


int main() {
    sf::RenderWindow window(sf::VideoMode(W * PIXEL_SIZE, H * PIXEL_SIZE), "sand");
//    window.setFramerateLimit(120);

    sf::Clock clock;

    ParticleSystem particles;
//    particles.fillHalf();

    // run the main loop
    long lc = 0, ms = 0;
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

        auto mouse = sf::Mouse::getPosition(window);
        particles.handleMouse(window.mapPixelToCoords(mouse), wheelDelta);

        auto elapsed = clock.restart();
        particles.update(elapsed);

        ms += elapsed.asMicroseconds();
        lc++;
        if (ms >= 1000000) { // show fps every second
            int fps = 1000000 / ((float) ms / lc);
            cout << fps << " fps" << endl;
            ms = lc = 0;
        }

        window.clear();
        window.draw(particles);
        window.display();
    }
    return 0;
}
