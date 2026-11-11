#pragma once

#include <cmath>

#include "mesh.h"
#include "math3d.h"

// ============================================================
// CREAR ESFERA LOW-POLY
// ============================================================
//
// latitudeSegments:
// cantidad de divisiones verticales.
//
// longitudeSegments:
// cantidad de divisiones alrededor.
//
// radio:
// radio de la esfera.
//
// Esta geometria servira para:
//
// - Sol
// - Planetas
// - Lunas
//
// Ejemplo:
//
// Mesh planet = createSphere(16, 24, 1.0f);
// ============================================================

inline Mesh createSphere(
    int latitudeSegments,
    int longitudeSegments,
    float radius
) {
    Mesh mesh;

    if (
        latitudeSegments < 2
        ||
        longitudeSegments < 3
        ||
        radius <= 0.0f
    ) {
        return mesh;
    }

    // ========================================================
    // VERTICES
    // ========================================================

    for (
        int lat = 0;
        lat <= latitudeSegments;
        lat++
    ) {
        float v =
            static_cast<float>(lat)
            /
            static_cast<float>(latitudeSegments);

        float theta =
            v
            *
            PI;

        float sinTheta =
            std::sin(theta);

        float cosTheta =
            std::cos(theta);

        for (
            int lon = 0;
            lon <= longitudeSegments;
            lon++
        ) {
            float u =
                static_cast<float>(lon)
                /
                static_cast<float>(longitudeSegments);

            float phi =
                u
                *
                2.0f
                *
                PI;

            float sinPhi =
                std::sin(phi);

            float cosPhi =
                std::cos(phi);

            Vertex vertex;

            vertex.position.x =
                radius
                *
                sinTheta
                *
                cosPhi;

            vertex.position.y =
                radius
                *
                sinTheta
                *
                sinPhi;

            vertex.position.z =
                radius
                *
                cosTheta;

            mesh.vertices.push_back(
                vertex
            );
        }
    }

    // ========================================================
    // CARAS
    // ========================================================

    int rowSize =
        longitudeSegments
        +
        1;

    for (
        int lat = 0;
        lat < latitudeSegments;
        lat++
    ) {
        for (
            int lon = 0;
            lon < longitudeSegments;
            lon++
        ) {
            int current =
                lat
                *
                rowSize
                +
                lon;

            int next =
                current
                +
                rowSize;

            // ------------------------------------------------
            // TRIANGULO 1
            // ------------------------------------------------

            Face face1 {
                current,
                next,
                current + 1
            };

            mesh.faces.push_back(
                face1
            );

            // ------------------------------------------------
            // TRIANGULO 2
            // ------------------------------------------------

            Face face2 {
                current + 1,
                next,
                next + 1
            };

            mesh.faces.push_back(
                face2
            );
        }
    }

    return mesh;
}

// ============================================================
// CREAR ANILLO
// ============================================================
//
// innerRadius:
// radio interior.
//
// outerRadius:
// radio exterior.
//
// segments:
// cantidad de divisiones.
//
// thickness:
// grosor vertical.
//
// Este mesh servira para crear un planeta tipo Saturno.
// ============================================================

inline Mesh createRing(
    int segments,
    float innerRadius,
    float outerRadius,
    float thickness
) {
    Mesh mesh;

    if (
        segments < 3
        ||
        innerRadius <= 0.0f
        ||
        outerRadius <= innerRadius
    ) {
        return mesh;
    }

    float halfThickness =
        thickness
        *
        0.5f;

    // ========================================================
    // VERTICES
    // ========================================================

    for (
        int i = 0;
        i <= segments;
        i++
    ) {
        float t =
            static_cast<float>(i)
            /
            static_cast<float>(segments);

        float angle =
            t
            *
            2.0f
            *
            PI;

        float c =
            std::cos(angle);

        float s =
            std::sin(angle);

        // ----------------------------------------------------
        // SUPERFICIE SUPERIOR
        // ----------------------------------------------------

        Vertex innerTop;
        innerTop.position = {
            c * innerRadius,
            s * innerRadius,
            halfThickness
        };

        Vertex outerTop;
        outerTop.position = {
            c * outerRadius,
            s * outerRadius,
            halfThickness
        };

        // ----------------------------------------------------
        // SUPERFICIE INFERIOR
        // ----------------------------------------------------

        Vertex innerBottom;
        innerBottom.position = {
            c * innerRadius,
            s * innerRadius,
            -halfThickness
        };

        Vertex outerBottom;
        outerBottom.position = {
            c * outerRadius,
            s * outerRadius,
            -halfThickness
        };

        mesh.vertices.push_back(
            innerTop
        );

        mesh.vertices.push_back(
            outerTop
        );

        mesh.vertices.push_back(
            innerBottom
        );

        mesh.vertices.push_back(
            outerBottom
        );
    }

    // ========================================================
    // CARAS
    // ========================================================

    for (
        int i = 0;
        i < segments;
        i++
    ) {
        int base =
            i
            *
            4;

        int next =
            (i + 1)
            *
            4;

        // ----------------------------------------------------
        // PARTE SUPERIOR
        // ----------------------------------------------------

        mesh.faces.push_back({
            base,
            next,
            base + 1
        });

        mesh.faces.push_back({
            base + 1,
            next,
            next + 1
        });

        // ----------------------------------------------------
        // PARTE INFERIOR
        // ----------------------------------------------------

        mesh.faces.push_back({
            base + 2,
            base + 3,
            next + 2
        });

        mesh.faces.push_back({
            base + 3,
            next + 3,
            next + 2
        });

        // ----------------------------------------------------
        // BORDE EXTERIOR
        // ----------------------------------------------------

        mesh.faces.push_back({
            base + 1,
            next + 1,
            base + 3
        });

        mesh.faces.push_back({
            base + 3,
            next + 1,
            next + 3
        });

        // ----------------------------------------------------
        // BORDE INTERIOR
        // ----------------------------------------------------

        mesh.faces.push_back({
            base,
            base + 2,
            next
        });

        mesh.faces.push_back({
            base + 2,
            next + 2,
            next
        });
    }

    return mesh;
}

// ============================================================
// CREAR CUBO SIMPLE
// ============================================================
//
// Lo usaremos mas adelante para objetos de superficie:
//
// - edificios
// - monumentos
// - rocas cuadradas
//
// De esta manera no necesitamos importar un OBJ para cada
// objeto sencillo.
// ============================================================

inline Mesh createCube(
    float size
) {
    Mesh mesh;

    float h =
        size
        *
        0.5f;

    mesh.vertices = {
        {{-h, -h, -h}},
        {{ h, -h, -h}},
        {{ h,  h, -h}},
        {{-h,  h, -h}},

        {{-h, -h,  h}},
        {{ h, -h,  h}},
        {{ h,  h,  h}},
        {{-h,  h,  h}}
    };

    mesh.faces = {
        // Abajo
        {0, 2, 1},
        {0, 3, 2},

        // Arriba
        {4, 5, 6},
        {4, 6, 7},

        // Frente
        {0, 1, 5},
        {0, 5, 4},

        // Atras
        {3, 7, 6},
        {3, 6, 2},

        // Izquierda
        {0, 4, 7},
        {0, 7, 3},

        // Derecha
        {1, 2, 6},
        {1, 6, 5}
    };

    return mesh;
}