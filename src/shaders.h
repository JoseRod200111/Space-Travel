#pragma once

#include <algorithm>
#include <cmath>

#include "color.h"
#include "math3d.h"

// ============================================================
// DATOS DEL FRAGMENT SHADER
// ============================================================

struct FragmentInput {
    Vec3 worldPosition;
    Vec3 normal;

    float time;

    Color baseColor;
};

// ============================================================
// UTILIDADES
// ============================================================

inline float clamp01(
    float value
) {
    return std::clamp(
        value,
        0.0f,
        1.0f
    );
}

// Parte decimal de un numero.
//
// Ejemplo:
// 3.75 -> 0.75
//
// Es mucho mas barato que usar sin() o cos().
inline float fractFast(
    float value
) {
    return value
        -
        std::floor(
            value
        );
}

// Onda triangular sin trigonometria.
//
// Resultado aproximado:
//
// 0 -> 1 -> 0
//
// Se utiliza para generar patrones y animaciones
// evitando sin() y cos().
inline float triangleWave(
    float value
) {
    float f =
        fractFast(
            value
        );

    return 1.0f
        -
        std::abs(
            f * 2.0f
            -
            1.0f
        );
}

// ============================================================
// COLOR
// ============================================================

inline Color applyIntensity(
    Color color,
    float intensity
) {
    intensity =
        clamp01(
            intensity
        );

    return Color(
        static_cast<unsigned char>(
            static_cast<float>(
                color.r
            )
            *
            intensity
        ),

        static_cast<unsigned char>(
            static_cast<float>(
                color.g
            )
            *
            intensity
        ),

        static_cast<unsigned char>(
            static_cast<float>(
                color.b
            )
            *
            intensity
        ),

        color.a
    );
}

// ============================================================
// MEZCLA DE COLORES
// ============================================================

inline Color mixColors(
    Color a,
    Color b,
    float t
) {
    t =
        clamp01(
            t
        );

    return Color(
        static_cast<unsigned char>(
            static_cast<float>(
                a.r
            )
            +
            (
                static_cast<float>(
                    b.r
                )
                -
                static_cast<float>(
                    a.r
                )
            )
            *
            t
        ),

        static_cast<unsigned char>(
            static_cast<float>(
                a.g
            )
            +
            (
                static_cast<float>(
                    b.g
                )
                -
                static_cast<float>(
                    a.g
                )
            )
            *
            t
        ),

        static_cast<unsigned char>(
            static_cast<float>(
                a.b
            )
            +
            (
                static_cast<float>(
                    b.b
                )
                -
                static_cast<float>(
                    a.b
                )
            )
            *
            t
        ),

        static_cast<unsigned char>(
            static_cast<float>(
                a.a
            )
            +
            (
                static_cast<float>(
                    b.a
                )
                -
                static_cast<float>(
                    a.a
                )
            )
            *
            t
        )
    );
}

// ============================================================
// ILUMINACION BASICA
// ============================================================
//
// Sin trigonometria.
//
// Solo:
// - normalizacion
// - producto punto
// - multiplicaciones
//
// La luz ambiental evita caras completamente negras.
// ============================================================

inline float basicLighting(
    Vec3 normal
) {
    const Vec3 lightDirection =
        normalize({
            -0.45f,
            -0.65f,
            1.0f
        });

    Vec3 n =
        normalize(
            normal
        );

    float diffuse =
        dot(
            n,
            lightDirection
        );

    diffuse =
        std::max(
            diffuse,
            0.0f
        );

    return clamp01(
        0.30f
        +
        diffuse
        *
        0.70f
    );
}

// ============================================================
// SHADER NAVE
// ============================================================
//
// Mantiene la apariencia azul mejorada,
// pero evita patrones trigonométricos por fragmento.
// ============================================================

inline Color spaceshipShader(
    const FragmentInput& input
) {
    Vec3 n =
        normalize(
            input.normal
        );

    const Vec3 lightDirection =
        normalize({
            -0.35f,
            -0.65f,
            1.0f
        });

    float diffuse =
        std::max(
            dot(
                n,
                lightDirection
            ),
            0.0f
        );

    float lighting =
        clamp01(
            0.55f
            +
            diffuse
            *
            0.45f
        );

    float topFacing =
        clamp01(
            std::abs(
                n.z
            )
        );

    float sideFacing =
        clamp01(
            std::abs(
                n.x
            )
        );

    Color darkBlue(
        55,
        105,
        175,
        255
    );

    Color mediumBlue(
        75,
        145,
        225,
        255
    );

    Color brightBlue(
        125,
        205,
        255,
        255
    );

    Color base =
        mixColors(
            darkBlue,
            mediumBlue,
            sideFacing
                *
                0.55f
        );

    base =
        mixColors(
            base,
            brightBlue,
            topFacing
                *
                0.65f
        );

    // Patrón metálico barato.
    //
    // Antes:
    // sin(x * 5 + y * 3)
    //
    // Ahora:
    // cuadrícula basada en floor().
    int metalCell =
        static_cast<int>(
            std::floor(
                input.worldPosition.x
                *
                4.0f
            )
        )
        +
        static_cast<int>(
            std::floor(
                input.worldPosition.y
                *
                4.0f
            )
        );

    if (
        metalCell % 5
        ==
        0
    ) {
        base =
            mixColors(
                base,
                brightBlue,
                0.12f
            );
    }

    return applyIntensity(
        base,
        lighting
    );
}

// ============================================================
// PLANETA ROCOSO
// ============================================================
//
// Antes utilizaba varios sin() y cos() por pixel.
//
// Ahora utilizamos una cuadricula irregular basada en
// coordenadas del mundo.
//
// Visualmente mantiene zonas claras y oscuras.
// ============================================================

inline Color rockyPlanetShader(
    const FragmentInput& input
) {
    float light =
        basicLighting(
            input.normal
        );

    int xCell =
        static_cast<int>(
            std::floor(
                input.worldPosition.x
                *
                4.0f
            )
        );

    int yCell =
        static_cast<int>(
            std::floor(
                input.worldPosition.y
                *
                4.0f
            )
        );

    int zCell =
        static_cast<int>(
            std::floor(
                input.worldPosition.z
                *
                4.0f
            )
        );

    int pattern =
        xCell * 3
        +
        yCell * 5
        +
        zCell * 7;

    Color darkRock(
        96,
        76,
        64,
        255
    );

    Color mediumRock(
        145,
        120,
        95,
        255
    );

    Color lightRock(
        185,
        160,
        125,
        255
    );

    Color color;

    int selector =
        std::abs(
            pattern
        )
        %
        7;

    if (
        selector <= 1
    ) {
        color =
            darkRock;
    }
    else if (
        selector <= 4
    ) {
        color =
            mediumRock;
    }
    else {
        color =
            lightRock;
    }

    return applyIntensity(
        color,
        light
    );
}

// ============================================================
// PLANETA VERDE
// ============================================================
//
// Conserva regiones verdes y azuladas,
// pero sin trigonometria.
// ============================================================

inline Color greenPlanetShader(
    const FragmentInput& input
) {
    float light =
        basicLighting(
            input.normal
        );

    int xCell =
        static_cast<int>(
            std::floor(
                input.worldPosition.x
                *
                3.0f
            )
        );

    int yCell =
        static_cast<int>(
            std::floor(
                input.worldPosition.y
                *
                3.0f
            )
        );

    int zCell =
        static_cast<int>(
            std::floor(
                input.worldPosition.z
                *
                5.0f
            )
        );

    int pattern =
        xCell
        +
        yCell * 2
        +
        zCell * 3;

    int selector =
        std::abs(
            pattern
        )
        %
        10;

    Color deepOcean(
        20,
        75,
        105,
        255
    );

    Color shallowWater(
        30,
        115,
        125,
        255
    );

    Color greenLand(
        45,
        145,
        75,
        255
    );

    Color brightLand(
        75,
        175,
        85,
        255
    );

    Color color;

    if (
        selector <= 2
    ) {
        color =
            deepOcean;
    }
    else if (
        selector <= 4
    ) {
        color =
            shallowWater;
    }
    else if (
        selector <= 8
    ) {
        color =
            greenLand;
    }
    else {
        color =
            brightLand;
    }

    return applyIntensity(
        color,
        light
    );
}

// ============================================================
// SOL ANIMADO
// ============================================================
//
// Sigue siendo un SHADER ANIMADO.
//
// Antes:
// 3 funciones trigonométricas POR PIXEL.
//
// Ahora:
// ondas triangulares usando:
// floor()
// abs()
// multiplicaciones
//
// El patrón todavía cambia con el tiempo.
// ============================================================

inline Color sunShader(
    const FragmentInput& input
) {
    float wave1 =
        triangleWave(
            input.worldPosition.x
            *
            1.8f
            +
            input.time
            *
            0.45f
        );

    float wave2 =
        triangleWave(
            input.worldPosition.y
            *
            2.2f
            -
            input.time
            *
            0.32f
        );

    float wave3 =
        triangleWave(
            input.worldPosition.z
            *
            1.5f
            +
            input.time
            *
            0.38f
        );

    float pattern =
        (
            wave1
            +
            wave2
            +
            wave3
        )
        /
        3.0f;

    Color deepOrange(
        255,
        75,
        10,
        255
    );

    Color orange(
        255,
        145,
        20,
        255
    );

    Color yellow(
        255,
        235,
        70,
        255
    );

    if (
        pattern
        <
        0.45f
    ) {
        return mixColors(
            deepOrange,
            orange,
            pattern
            /
            0.45f
        );
    }

    return mixColors(
        orange,
        yellow,
        (
            pattern
            -
            0.45f
        )
        /
        0.55f
    );
}

// ============================================================
// ANILLOS
// ============================================================
//
// Antes:
// sqrt() + sin() por fragmento.
//
// Ahora:
// utilizamos radio AL CUADRADO.
//
// Para distinguir bandas no necesitamos la distancia exacta.
// ============================================================

inline Color ringShader(
    const FragmentInput& input
) {
    float x =
        input.worldPosition.x;

    float y =
        input.worldPosition.y;

    float radiusSquared =
        x * x
        +
        y * y;

    int band =
        static_cast<int>(
            radiusSquared
            *
            10.0f
        );

    Color brightRing(
        220,
        195,
        145,
        255
    );

    Color mediumRing(
        175,
        145,
        100,
        255
    );

    Color darkRing(
        125,
        100,
        70,
        255
    );

    Color color;

    int selector =
        std::abs(
            band
        )
        %
        6;

    if (
        selector <= 1
    ) {
        color =
            brightRing;
    }
    else if (
        selector <= 3
    ) {
        color =
            mediumRing;
    }
    else {
        color =
            darkRing;
    }

    float light =
        basicLighting(
            input.normal
        );

    light =
        0.50f
        +
        light
        *
        0.50f;

    return applyIntensity(
        color,
        light
    );
}