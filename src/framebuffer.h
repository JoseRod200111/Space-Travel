#pragma once

#include <SDL2/SDL.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <vector>

#include "color.h"

// ============================================================
// RESOLUCION
// ============================================================

constexpr int SCREEN_WIDTH = 960;
constexpr int SCREEN_HEIGHT = 720;

// ============================================================
// FRAMEBUFFER
// ============================================================

inline std::vector<Color> framebuffer(
    SCREEN_WIDTH * SCREEN_HEIGHT,
    Color(
        0,
        0,
        0,
        255
    )
);

// ============================================================
// Z-BUFFER
// ============================================================
//
// Guarda la profundidad del pixel mas cercano
// que ya fue dibujado.
//
// Al comenzar cada frame se llena con infinito,
// lo que significa que todavia no hay ningun objeto
// dibujado en ese pixel.
// ============================================================

inline std::vector<float> depthBuffer(
    SCREEN_WIDTH * SCREEN_HEIGHT,
    std::numeric_limits<float>::infinity()
);

// ============================================================
// COLOR ACTUAL
// ============================================================

inline Color currentColor(
    255,
    255,
    255,
    255
);

// ============================================================
// SET COLOR
// ============================================================

inline void setColor(
    const Color& color
) {
    currentColor =
        color;
}

// ============================================================
// CLEAR
// ============================================================
//
// Limpia tanto:
// - framebuffer
// - Z-buffer
//
// Esto debe ejecutarse al comenzar cada frame.
// ============================================================

inline void clear(
    const Color& backgroundColor =
        Color(
            6,
            8,
            18,
            255
        )
) {
    std::fill(
        framebuffer.begin(),
        framebuffer.end(),
        backgroundColor
    );

    std::fill(
        depthBuffer.begin(),
        depthBuffer.end(),
        std::numeric_limits<float>::infinity()
    );
}

// ============================================================
// POINT SIN PROFUNDIDAD
// ============================================================
//
// Lo conservamos porque nos servira para elementos
// como HUD, estrellas simples u otras cosas 2D.
// ============================================================

inline void point(
    int x,
    int y
) {
    if (
        x < 0
        ||
        x >= SCREEN_WIDTH
        ||
        y < 0
        ||
        y >= SCREEN_HEIGHT
    ) {
        return;
    }

    framebuffer[
        y * SCREEN_WIDTH + x
    ] =
        currentColor;
}

inline void point(
    int x,
    int y,
    const Color& color
) {
    if (
        x < 0
        ||
        x >= SCREEN_WIDTH
        ||
        y < 0
        ||
        y >= SCREEN_HEIGHT
    ) {
        return;
    }

    framebuffer[
        y * SCREEN_WIDTH + x
    ] =
        color;
}

// ============================================================
// DEPTH TEST
// ============================================================
//
// depth esta normalizado aproximadamente entre:
//
// 0.0 = cerca
// 1.0 = lejos
//
// Si el nuevo fragmento esta mas cerca que el valor
// almacenado, gana y actualiza el Z-buffer.
// ============================================================

inline bool depthTest(
    int x,
    int y,
    float depth
) {
    if (
        x < 0
        ||
        x >= SCREEN_WIDTH
        ||
        y < 0
        ||
        y >= SCREEN_HEIGHT
    ) {
        return false;
    }

    if (
        depth < 0.0f
        ||
        depth > 1.0f
    ) {
        return false;
    }

    int index =
        y * SCREEN_WIDTH + x;

    if (
        depth
        <
        depthBuffer[index]
    ) {
        depthBuffer[index] =
            depth;

        return true;
    }

    return false;
}

// ============================================================
// POINT CON PROFUNDIDAD
// ============================================================

inline void pointDepth(
    int x,
    int y,
    float depth,
    const Color& color
) {
    if (
        depthTest(
            x,
            y,
            depth
        )
    ) {
        framebuffer[
            y * SCREEN_WIDTH + x
        ] =
            color;
    }
}

// ============================================================
// LEER COLOR
// ============================================================
//
// Puede servir despues para efectos o depuracion.
// ============================================================

inline Color getColor(
    int x,
    int y
) {
    if (
        x < 0
        ||
        x >= SCREEN_WIDTH
        ||
        y < 0
        ||
        y >= SCREEN_HEIGHT
    ) {
        return Color(
            0,
            0,
            0,
            255
        );
    }

    return framebuffer[
        y * SCREEN_WIDTH + x
    ];
}

// ============================================================
// LEER PROFUNDIDAD
// ============================================================

inline float getDepth(
    int x,
    int y
) {
    if (
        x < 0
        ||
        x >= SCREEN_WIDTH
        ||
        y < 0
        ||
        y >= SCREEN_HEIGHT
    ) {
        return
            std::numeric_limits<float>::infinity();
    }

    return depthBuffer[
        y * SCREEN_WIDTH + x
    ];
}

// ============================================================
// RENDER BUFFER
// ============================================================
//
// Convierte nuestro framebuffer de software
// en una textura de SDL y la muestra.
// ============================================================

inline void RenderBuffer(
    SDL_Renderer* renderer
) {
    static SDL_Texture* texture =
        nullptr;

    if (
        texture == nullptr
    ) {
        texture =
            SDL_CreateTexture(
                renderer,
                SDL_PIXELFORMAT_ABGR8888,
                SDL_TEXTUREACCESS_STREAMING,
                SCREEN_WIDTH,
                SCREEN_HEIGHT
            );
    }

    if (
        texture == nullptr
    ) {
        return;
    }

    void* pixels =
        nullptr;

    int pitch =
        0;

    if (
        SDL_LockTexture(
            texture,
            nullptr,
            &pixels,
            &pitch
        )
        !=
        0
    ) {
        return;
    }

    for (
        int y = 0;
        y < SCREEN_HEIGHT;
        y++
    ) {
        std::memcpy(
            static_cast<unsigned char*>(
                pixels
            )
                +
                y * pitch,

            framebuffer.data()
                +
                y * SCREEN_WIDTH,

            SCREEN_WIDTH
                *
                sizeof(Color)
        );
    }

    SDL_UnlockTexture(
        texture
    );

    SDL_RenderClear(
        renderer
    );

    SDL_RenderCopy(
        renderer,
        texture,
        nullptr,
        nullptr
    );
}