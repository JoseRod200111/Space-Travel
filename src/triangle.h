#pragma once

#include <algorithm>
#include <cmath>

#include "framebuffer.h"

// ============================================================
// VERTICE YA PROYECTADO A PANTALLA
// ============================================================

struct ScreenVertex {
    float x;
    float y;
    float depth;
};

// ============================================================
// EDGE FUNCTION
// ============================================================
//
// Determina en qué lado de una arista se encuentra un punto.
//
// También se utiliza para calcular las coordenadas
// baricéntricas del triángulo.
// ============================================================

inline float edgeFunction(
    const ScreenVertex& a,
    const ScreenVertex& b,
    float px,
    float py
) {
    return
        (
            px - a.x
        )
        *
        (
            b.y - a.y
        )
        -
        (
            py - a.y
        )
        *
        (
            b.x - a.x
        );
}

// ============================================================
// RASTERIZADOR DE TRIANGULOS
// ============================================================
//
// Esta versión mantiene:
//
// - coordenadas baricéntricas
// - interpolación de profundidad
// - fragment shader
// - Z-buffer
// - framebuffer
//
// OPTIMIZACION:
//
// En vez de recalcular desde cero:
//
// edgeFunction(...)
//
// tres veces PARA CADA PIXEL,
// calculamos los valores iniciales y los vamos actualizando
// incrementalmente con sumas.
//
// Esto reduce bastante el trabajo del rasterizador.
// ============================================================

template <typename FragmentShader>
inline void triangle(
    const ScreenVertex& v0,
    const ScreenVertex& v1,
    const ScreenVertex& v2,
    FragmentShader fragmentShader
) {
    // ========================================================
    // AREA DEL TRIANGULO
    // ========================================================

    float area =
        edgeFunction(
            v0,
            v1,
            v2.x,
            v2.y
        );

    // Triángulo degenerado.
    if (
        std::abs(area)
        <
        0.000001f
    ) {
        return;
    }

    float inverseArea =
        1.0f
        /
        area;

    // ========================================================
    // BOUNDING BOX
    // ========================================================

    float minXF =
        std::min({
            v0.x,
            v1.x,
            v2.x
        });

    float maxXF =
        std::max({
            v0.x,
            v1.x,
            v2.x
        });

    float minYF =
        std::min({
            v0.y,
            v1.y,
            v2.y
        });

    float maxYF =
        std::max({
            v0.y,
            v1.y,
            v2.y
        });

    int minX =
        std::max(
            0,
            static_cast<int>(
                std::floor(
                    minXF
                )
            )
        );

    int maxX =
        std::min(
            SCREEN_WIDTH - 1,
            static_cast<int>(
                std::ceil(
                    maxXF
                )
            )
        );

    int minY =
        std::max(
            0,
            static_cast<int>(
                std::floor(
                    minYF
                )
            )
        );

    int maxY =
        std::min(
            SCREEN_HEIGHT - 1,
            static_cast<int>(
                std::ceil(
                    maxYF
                )
            )
        );

    // Triángulo completamente fuera de pantalla.
    if (
        minX > maxX
        ||
        minY > maxY
    ) {
        return;
    }

    // ========================================================
    // INCREMENTOS DE EDGE FUNCTIONS
    // ========================================================
    //
    // Para:
    //
    // E(a,b,p)
    //
    // mover un pixel en X cambia E por:
    //
    // b.y - a.y
    //
    // mover un pixel en Y cambia E por:
    //
    // -(b.x - a.x)
    //
    // Por eso podemos recorrer todo el triángulo
    // usando principalmente sumas.
    // ========================================================

    const float e0StepX =
        v2.y
        -
        v1.y;

    const float e0StepY =
        -(
            v2.x
            -
            v1.x
        );

    const float e1StepX =
        v0.y
        -
        v2.y;

    const float e1StepY =
        -(
            v0.x
            -
            v2.x
        );

    const float e2StepX =
        v1.y
        -
        v0.y;

    const float e2StepY =
        -(
            v1.x
            -
            v0.x
        );

    // ========================================================
    // PRIMER PIXEL
    // ========================================================
    //
    // Usamos el centro del pixel:
    //
    // x + 0.5
    // y + 0.5
    // ========================================================

    float startPX =
        static_cast<float>(
            minX
        )
        +
        0.5f;

    float startPY =
        static_cast<float>(
            minY
        )
        +
        0.5f;

    float rowE0 =
        edgeFunction(
            v1,
            v2,
            startPX,
            startPY
        );

    float rowE1 =
        edgeFunction(
            v2,
            v0,
            startPX,
            startPY
        );

    float rowE2 =
        edgeFunction(
            v0,
            v1,
            startPX,
            startPY
        );

    // ========================================================
    // ORIENTACION DEL TRIANGULO
    // ========================================================

    bool positiveArea =
        area
        >
        0.0f;

    constexpr float tolerance =
        0.00001f;

    // ========================================================
    // RASTERIZACION
    // ========================================================

    for (
        int y = minY;
        y <= maxY;
        y++
    ) {
        float e0 =
            rowE0;

        float e1 =
            rowE1;

        float e2 =
            rowE2;

        for (
            int x = minX;
            x <= maxX;
            x++
        ) {
            // =================================================
            // ¿PIXEL DENTRO DEL TRIANGULO?
            // =================================================
            //
            // Primero verificamos los edge values SIN hacer
            // divisiones.
            //
            // Solo si el pixel está dentro calculamos los
            // pesos baricéntricos.
            // =================================================

            bool inside;

            if (
                positiveArea
            ) {
                inside =
                    e0 >= -tolerance
                    &&
                    e1 >= -tolerance
                    &&
                    e2 >= -tolerance;
            }
            else {
                inside =
                    e0 <= tolerance
                    &&
                    e1 <= tolerance
                    &&
                    e2 <= tolerance;
            }

            if (
                inside
            ) {
                // =============================================
                // COORDENADAS BARICENTRICAS
                // =============================================

                float w0 =
                    e0
                    *
                    inverseArea;

                float w1 =
                    e1
                    *
                    inverseArea;

                float w2 =
                    e2
                    *
                    inverseArea;

                // =============================================
                // PROFUNDIDAD
                // =============================================

                float depth =
                    w0 * v0.depth
                    +
                    w1 * v1.depth
                    +
                    w2 * v2.depth;

                // =============================================
                // FRAGMENT SHADER
                // =============================================

                Color fragmentColor =
                    fragmentShader(
                        w0,
                        w1,
                        w2
                    );

                // =============================================
                // Z-BUFFER + FRAMEBUFFER
                // =============================================

                pointDepth(
                    x,
                    y,
                    depth,
                    fragmentColor
                );
            }

            // =================================================
            // SIGUIENTE PIXEL EN X
            // =================================================

            e0 +=
                e0StepX;

            e1 +=
                e1StepX;

            e2 +=
                e2StepX;
        }

        // =====================================================
        // SIGUIENTE FILA
        // =====================================================

        rowE0 +=
            e0StepY;

        rowE1 +=
            e1StepY;

        rowE2 +=
            e2StepY;
    }
}