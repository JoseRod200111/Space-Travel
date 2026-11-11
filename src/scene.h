#pragma once

#include <cmath>
#include <string>

#include "color.h"
#include "math3d.h"
#include "mesh.h"

// ============================================================
// TIPO DE SHADER
// ============================================================
//
// Nos permite indicar qué shader debe utilizar cada objeto.
//
// Después, durante el render:
//
// ShaderType::Spaceship
//      -> spaceshipShader()
//
// ShaderType::Sun
//      -> sunShader()
//
// etc.
// ============================================================

enum class ShaderType {
    Spaceship,
    Sun,
    RockyPlanet,
    GreenPlanet,
    Rings,
    FlatColor
};

// ============================================================
// TRANSFORM
// ============================================================
//
// Cada objeto 3D tiene:
//
// position
// rotation
// scale
//
// rotation se guarda en radianes.
// ============================================================

struct Transform {
    Vec3 position {
        0.0f,
        0.0f,
        0.0f
    };

    Vec3 rotation {
        0.0f,
        0.0f,
        0.0f
    };

    Vec3 scale {
        1.0f,
        1.0f,
        1.0f
    };
};

// ============================================================
// OBJETO DE ESCENA
// ============================================================
//
// Esta estructura servirá para:
//
// - Sol
// - planetas
// - lunas
// - anillos
// - nave
// - objetos sobre superficies
// ============================================================

struct SceneObject {
    std::string name;

    const Mesh* mesh =
        nullptr;

    Transform transform;

    ShaderType shaderType =
        ShaderType::FlatColor;

    Color baseColor {
        255,
        255,
        255,
        255
    };

    // --------------------------------------------------------
    // COLISION
    // --------------------------------------------------------

    float collisionRadius =
        0.0f;

    bool collisionEnabled =
        false;

    // --------------------------------------------------------
    // ROTACION SOBRE SU PROPIO EJE
    // --------------------------------------------------------

    float rotationSpeed =
        0.0f;

    float rotationPhase =
        0.0f;

    // --------------------------------------------------------
    // ORBITA
    // --------------------------------------------------------

    bool orbitEnabled =
        false;

    float orbitRadius =
        0.0f;

    float orbitSpeed =
        0.0f;

    float orbitPhase =
        0.0f;

    float orbitHeight =
        0.0f;

    // --------------------------------------------------------
    // ESTADO
    // --------------------------------------------------------

    bool visible =
        true;
};

// ============================================================
// MATRIZ MODEL
// ============================================================
//
// Orden:
//
// Scale
// Rotation X
// Rotation Y
// Rotation Z
// Translation
//
// Como multiplicamos matrices de derecha a izquierda:
//
// Model = T * Rz * Ry * Rx * S
// ============================================================

inline Mat4 getModelMatrix(
    const SceneObject& object
) {
    Mat4 scale =
        makeScale(
            object.transform.scale.x,
            object.transform.scale.y,
            object.transform.scale.z
        );

    Mat4 rotationX =
        makeRotationX(
            object.transform.rotation.x
        );

    Mat4 rotationY =
        makeRotationY(
            object.transform.rotation.y
        );

    Mat4 rotationZ =
        makeRotationZ(
            object.transform.rotation.z
        );

    Mat4 translation =
        makeTranslation(
            object.transform.position.x,
            object.transform.position.y,
            object.transform.position.z
        );

    return
        translation
        *
        rotationZ
        *
        rotationY
        *
        rotationX
        *
        scale;
}

// ============================================================
// ACTUALIZAR ROTACION
// ============================================================
//
// La rotacion NO depende del número de cuadros.
//
// Se calcula directamente usando:
//
// tiempo transcurrido * velocidad
//
// Esto hace que funcione igual aunque el programa corra
// a distintas cantidades de FPS.
// ============================================================

inline void updateRotation(
    SceneObject& object,
    float elapsedTime
) {
    object.transform.rotation.z =
        object.rotationPhase
        +
        elapsedTime
        *
        object.rotationSpeed;
}

// ============================================================
// ACTUALIZAR ORBITA
// ============================================================
//
// orbitCenter:
//
// posición del objeto alrededor del cual orbita.
//
// Para planetas normalmente será:
//
// {0, 0, 0}
//
// porque el Sol estará en el origen.
//
// Para una luna será la posición actual de su planeta.
// ============================================================

inline void updateOrbit(
    SceneObject& object,
    const Vec3& orbitCenter,
    float elapsedTime
) {
    if (
        !object.orbitEnabled
    ) {
        return;
    }

    float angle =
        object.orbitPhase
        +
        elapsedTime
        *
        object.orbitSpeed;

    object.transform.position.x =
        orbitCenter.x
        +
        std::cos(angle)
        *
        object.orbitRadius;

    object.transform.position.y =
        orbitCenter.y
        +
        std::sin(angle)
        *
        object.orbitRadius;

    object.transform.position.z =
        orbitCenter.z
        +
        object.orbitHeight;
}

// ============================================================
// ACTUALIZAR OBJETO ORBITAL
// ============================================================
//
// Actualiza:
//
// - órbita
// - rotación propia
//
// usando el mismo elapsedTime.
// ============================================================

inline void updateOrbitalObject(
    SceneObject& object,
    const Vec3& orbitCenter,
    float elapsedTime
) {
    updateOrbit(
        object,
        orbitCenter,
        elapsedTime
    );

    updateRotation(
        object,
        elapsedTime
    );
}

// ============================================================
// DISTANCIA ENTRE DOS PUNTOS
// ============================================================

inline float distanceBetween(
    const Vec3& a,
    const Vec3& b
) {
    return length(
        subtract(
            a,
            b
        )
    );
}

// ============================================================
// COLISION PUNTO - ESFERA
// ============================================================
//
// Esto nos servirá para evitar que:
//
// - la nave atraviese planetas
// - la cámara termine dentro de un planeta
//
// extraRadius permite agregar un pequeño margen.
// ============================================================

inline bool pointInsideObject(
    const Vec3& point,
    const SceneObject& object,
    float extraRadius = 0.0f
) {
    if (
        !object.collisionEnabled
    ) {
        return false;
    }

    float distance =
        distanceBetween(
            point,
            object.transform.position
        );

    float radius =
        object.collisionRadius
        +
        extraRadius;

    return
        distance
        <
        radius;
}

// ============================================================
// SACAR UN PUNTO FUERA DE UN PLANETA
// ============================================================
//
// Si una posición terminó dentro del planeta,
// la desplazamos hasta la superficie + margen.
//
// Más adelante será útil para resolver la colisión
// de la nave de forma sencilla.
// ============================================================

inline Vec3 pushOutsideObject(
    const Vec3& point,
    const SceneObject& object,
    float extraRadius = 0.0f
) {
    if (
        !object.collisionEnabled
    ) {
        return point;
    }

    Vec3 direction =
        subtract(
            point,
            object.transform.position
        );

    float distance =
        length(
            direction
        );

    float minimumDistance =
        object.collisionRadius
        +
        extraRadius;

    if (
        distance
        >=
        minimumDistance
    ) {
        return point;
    }

    if (
        distance
        <
        0.0001f
    ) {
        direction = {
            0.0f,
            0.0f,
            1.0f
        };
    }
    else {
        direction =
            normalize(
                direction
            );
    }

    return add(
        object.transform.position,

        multiply(
            direction,
            minimumDistance
        )
    );
}

// ============================================================
// CREAR OBJETO BASICO
// ============================================================
//
// Función auxiliar para evitar repetir mucha configuración
// cuando creemos el sistema solar.
// ============================================================

inline SceneObject createSceneObject(
    const std::string& name,
    const Mesh* mesh,
    ShaderType shaderType,
    Vec3 position,
    Vec3 scale
) {
    SceneObject object;

    object.name =
        name;

    object.mesh =
        mesh;

    object.shaderType =
        shaderType;

    object.transform.position =
        position;

    object.transform.scale =
        scale;

    return object;
}

// ============================================================
// CONFIGURAR ORBITA
// ============================================================

inline void configureOrbit(
    SceneObject& object,
    float radius,
    float speed,
    float phase = 0.0f,
    float height = 0.0f
) {
    object.orbitEnabled =
        true;

    object.orbitRadius =
        radius;

    object.orbitSpeed =
        speed;

    object.orbitPhase =
        phase;

    object.orbitHeight =
        height;
}

// ============================================================
// CONFIGURAR ROTACION
// ============================================================

inline void configureRotation(
    SceneObject& object,
    float speed,
    float phase = 0.0f
) {
    object.rotationSpeed =
        speed;

    object.rotationPhase =
        phase;
}

// ============================================================
// CONFIGURAR COLISION
// ============================================================

inline void configureCollision(
    SceneObject& object,
    float radius
) {
    object.collisionEnabled =
        true;

    object.collisionRadius =
        radius;
}