#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "mesh.h"

// ============================================================
// LEER INDICE DE OBJ
// ============================================================
//
// Un indice de una cara OBJ puede aparecer como:
//
// 1
// 1/2
// 1/2/3
// 1//3
//
// Solo necesitamos el primer numero:
// el indice del vertice.
//
// OBJ comienza sus indices en 1.
// std::vector comienza en 0.
// ============================================================

inline int readOBJIndex(
    const std::string& token,
    int vertexCount
) {
    std::size_t slashPosition =
        token.find('/');

    std::string number;

    if (
        slashPosition
        ==
        std::string::npos
    ) {
        number =
            token;
    }
    else {
        number =
            token.substr(
                0,
                slashPosition
            );
    }

    if (
        number.empty()
    ) {
        return -1;
    }

    int index =
        std::stoi(
            number
        );

    // --------------------------------------------------------
    // INDICE POSITIVO
    // --------------------------------------------------------
    //
    // OBJ:
    // 1 -> primer vertice
    //
    // C++:
    // 0 -> primer vertice
    // --------------------------------------------------------

    if (
        index > 0
    ) {
        return
            index - 1;
    }

    // --------------------------------------------------------
    // INDICE NEGATIVO
    // --------------------------------------------------------
    //
    // OBJ tambien permite:
    //
    // -1 = ultimo vertice
    // -2 = penultimo vertice
    // --------------------------------------------------------

    if (
        index < 0
    ) {
        return
            vertexCount
            +
            index;
    }

    return -1;
}

// ============================================================
// CARGAR OBJ
// ============================================================
//
// Lee:
//
// v = vertices
// f = caras
//
// El modelo de la nave ya fue exportado desde Blender
// utilizando Triangulated Mesh.
//
// Por eso normalmente cada cara contiene tres vertices.
//
// De todas formas, este loader tambien puede triangular
// una cara con mas de tres vertices utilizando un fan.
// ============================================================

inline bool loadOBJ(
    const std::string& filename,
    Mesh& mesh
) {
    std::ifstream file(
        filename
    );

    if (
        !file.is_open()
    ) {
        std::cout
            << "ERROR: No se pudo abrir el modelo: "
            << filename
            << std::endl;

        return false;
    }

    // Limpiar el mesh antes de cargar.
    mesh.vertices.clear();
    mesh.faces.clear();

    std::string line;

    int lineNumber =
        0;

    while (
        std::getline(
            file,
            line
        )
    ) {
        lineNumber++;

        if (
            line.empty()
        ) {
            continue;
        }

        // Ignorar comentarios.
        if (
            line[0]
            ==
            '#'
        ) {
            continue;
        }

        std::stringstream stream(
            line
        );

        std::string type;

        stream >>
            type;

        // ====================================================
        // VERTICE
        // ====================================================
        //
        // Ejemplo:
        //
        // v 1.0 2.0 3.0
        // ====================================================

        if (
            type
            ==
            "v"
        ) {
            Vertex vertex;

            if (
                !(
                    stream
                    >>
                    vertex.position.x
                    >>
                    vertex.position.y
                    >>
                    vertex.position.z
                )
            ) {
                std::cout
                    << "Advertencia: vertice invalido en linea "
                    << lineNumber
                    << std::endl;

                continue;
            }

            mesh.vertices.push_back(
                vertex
            );
        }

        // ====================================================
        // CARA
        // ====================================================
        //
        // Ejemplos:
        //
        // f 1 2 3
        //
        // f 1/1/1 2/2/2 3/3/3
        //
        // Nuestro modelo ya esta triangulado.
        // ====================================================

        else if (
            type
            ==
            "f"
        ) {
            std::string token;

            std::vector<int>
                indices;

            while (
                stream
                >>
                token
            ) {
                int index =
                    readOBJIndex(
                        token,
                        static_cast<int>(
                            mesh.vertices.size()
                        )
                    );

                if (
                    index >= 0
                    &&
                    index <
                        static_cast<int>(
                            mesh.vertices.size()
                        )
                ) {
                    indices.push_back(
                        index
                    );
                }
            }

            if (
                indices.size()
                <
                3
            ) {
                continue;
            }

            // -----------------------------------------------
            // TRIANGULACION TIPO FAN
            // -----------------------------------------------
            //
            // Para:
            //
            // 0 1 2 3
            //
            // genera:
            //
            // 0 1 2
            // 0 2 3
            //
            // En nuestra nave normalmente no sera necesario
            // porque Blender ya exporto todo triangulado.
            // -----------------------------------------------

            for (
                std::size_t i = 1;
                i + 1 < indices.size();
                i++
            ) {
                Face face {
                    indices[0],
                    indices[i],
                    indices[i + 1]
                };

                mesh.faces.push_back(
                    face
                );
            }
        }
    }

    file.close();

    // ========================================================
    // VERIFICACION
    // ========================================================

    if (
        mesh.vertices.empty()
    ) {
        std::cout
            << "ERROR: El modelo no contiene vertices."
            << std::endl;

        return false;
    }

    if (
        mesh.faces.empty()
    ) {
        std::cout
            << "ERROR: El modelo no contiene caras."
            << std::endl;

        return false;
    }

    MeshBounds bounds =
        calculateMeshBounds(
            mesh
        );

    // ========================================================
    // INFORMACION
    // ========================================================

    std::cout
        << "OBJ cargado correctamente:"
        << std::endl;

    std::cout
        << "  Archivo: "
        << filename
        << std::endl;

    std::cout
        << "  Vertices: "
        << mesh.vertices.size()
        << std::endl;

    std::cout
        << "  Triangulos: "
        << mesh.faces.size()
        << std::endl;

    std::cout
        << "  Tamano X: "
        << bounds.width
        << std::endl;

    std::cout
        << "  Tamano Y: "
        << bounds.height
        << std::endl;

    std::cout
        << "  Tamano Z: "
        << bounds.depth
        << std::endl;

    return true;
}