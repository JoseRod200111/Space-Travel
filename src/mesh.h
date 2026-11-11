#pragma once

#include <vector>

#include "math3d.h"

// ============================================================
// VERTICE DEL MODELO
// ============================================================
//
// Por ahora solo necesitamos posicion.
//
// Mas adelante podemos ampliar esta estructura con:
// - normal
// - coordenadas UV
// - otros atributos
//
// sin cambiar la estructura general del renderizador.
// ============================================================

struct Vertex {
    Vec3 position;
};

// ============================================================
// CARA TRIANGULAR
// ============================================================
//
// Cada cara contiene los indices de tres vertices.
//
// Ejemplo:
//
// a = 0
// b = 1
// c = 2
//
// representa:
//
// vertices[0]
// vertices[1]
// vertices[2]
// ============================================================

struct Face {
    int a;
    int b;
    int c;
};

// ============================================================
// MESH
// ============================================================
//
// Un Mesh contiene:
//
// - todos sus vertices
// - todas sus caras triangulares
//
// Esta misma estructura servira para:
//
// - nave cargada desde OBJ
// - planetas
// - sol
// - luna
// - anillos
// - objetos de superficie
// ============================================================

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
};

// ============================================================
// INFORMACION DE LIMITES
// ============================================================
//
// Nos sirve para:
//
// - centrar modelos importados
// - conocer su escala
// - calcular radios aproximados
// ============================================================

struct MeshBounds {
    Vec3 minimum;
    Vec3 maximum;
    Vec3 center;

    float width;
    float height;
    float depth;

    float largestSize;
};

// ============================================================
// CALCULAR LIMITES
// ============================================================

inline MeshBounds calculateMeshBounds(
    const Mesh& mesh
) {
    MeshBounds bounds {};

    if (
        mesh.vertices.empty()
    ) {
        bounds.minimum = {
            0.0f,
            0.0f,
            0.0f
        };

        bounds.maximum =
            bounds.minimum;

        bounds.center =
            bounds.minimum;

        bounds.width =
            0.0f;

        bounds.height =
            0.0f;

        bounds.depth =
            0.0f;

        bounds.largestSize =
            0.0f;

        return bounds;
    }

    Vec3 minimum =
        mesh.vertices[0].position;

    Vec3 maximum =
        mesh.vertices[0].position;

    for (
        const Vertex& vertex :
        mesh.vertices
    ) {
        const Vec3& p =
            vertex.position;

        if (
            p.x <
            minimum.x
        ) {
            minimum.x =
                p.x;
        }

        if (
            p.y <
            minimum.y
        ) {
            minimum.y =
                p.y;
        }

        if (
            p.z <
            minimum.z
        ) {
            minimum.z =
                p.z;
        }

        if (
            p.x >
            maximum.x
        ) {
            maximum.x =
                p.x;
        }

        if (
            p.y >
            maximum.y
        ) {
            maximum.y =
                p.y;
        }

        if (
            p.z >
            maximum.z
        ) {
            maximum.z =
                p.z;
        }
    }

    bounds.minimum =
        minimum;

    bounds.maximum =
        maximum;

    bounds.center = {
        (
            minimum.x
            +
            maximum.x
        )
        *
        0.5f,

        (
            minimum.y
            +
            maximum.y
        )
        *
        0.5f,

        (
            minimum.z
            +
            maximum.z
        )
        *
        0.5f
    };

    bounds.width =
        maximum.x
        -
        minimum.x;

    bounds.height =
        maximum.y
        -
        minimum.y;

    bounds.depth =
        maximum.z
        -
        minimum.z;

    bounds.largestSize =
        bounds.width;

    if (
        bounds.height
        >
        bounds.largestSize
    ) {
        bounds.largestSize =
            bounds.height;
    }

    if (
        bounds.depth
        >
        bounds.largestSize
    ) {
        bounds.largestSize =
            bounds.depth;
    }

    return bounds;
}