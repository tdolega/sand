//
// Created by tdolega on 14.04.2021.
//

#ifndef SAND_IMPORTSXD_H
#define SAND_IMPORTSXD_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <execution>
#include <algorithm>
#include <parallel/algorithm>
#include <thread>
#include <mutex>

using std::cout;
using std::endl;

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

#endif //SAND_IMPORTSXD_H
