#pragma once

#include <cstdint>

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    Color(
        uint8_t r = 0,
        uint8_t g = 0,
        uint8_t b = 0,
        uint8_t a = 255
    ) : r(r), g(g), b(b), a(a) {}
};
