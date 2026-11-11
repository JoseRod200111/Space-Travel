#pragma once

#include "math3d.h"

// ============================================================
// CAMARA 3D
// ============================================================
//
// La camara se define mediante:
//
// position -> donde esta la camara
// target   -> hacia donde mira
// up       -> cual direccion consideramos "arriba"
//
// Tambien guardamos:
//
// fov
// nearPlane
// farPlane
//
// para construir la matriz de proyeccion.
// ============================================================

struct Camera {
    Vec3 position {
        0.0f,
        -8.0f,
        4.0f
    };

    Vec3 target {
        0.0f,
        0.0f,
        0.0f
    };

    Vec3 up {
        0.0f,
        0.0f,
        1.0f
    };

    float fovDegrees =
        60.0f;

    float nearPlane =
        0.1f;

    float farPlane =
        500.0f;
};

// ============================================================
// DIRECCION DE LA CAMARA
// ============================================================

inline Vec3 getCameraForward(
    const Camera& camera
) {
    return normalize(
        subtract(
            camera.target,
            camera.position
        )
    );
}

// ============================================================
// VECTOR DERECHO DE LA CAMARA
// ============================================================

inline Vec3 getCameraRight(
    const Camera& camera
) {
    Vec3 forward =
        getCameraForward(
            camera
        );

    return normalize(
        cross(
            forward,
            camera.up
        )
    );
}

// ============================================================
// VECTOR ARRIBA REAL DE LA CAMARA
// ============================================================
//
// Lo calculamos usando forward y right para mantener
// los tres ejes perpendiculares.
// ============================================================

inline Vec3 getCameraUp(
    const Camera& camera
) {
    Vec3 forward =
        getCameraForward(
            camera
        );

    Vec3 right =
        getCameraRight(
            camera
        );

    return normalize(
        cross(
            right,
            forward
        )
    );
}

// ============================================================
// VIEW MATRIX
// ============================================================

inline Mat4 getViewMatrix(
    const Camera& camera
) {
    return makeLookAt(
        camera.position,
        camera.target,
        camera.up
    );
}

// ============================================================
// PROJECTION MATRIX
// ============================================================

inline Mat4 getProjectionMatrix(
    const Camera& camera,
    float aspectRatio
) {
    return makePerspective(
        radians(
            camera.fovDegrees
        ),

        aspectRatio,

        camera.nearPlane,

        camera.farPlane
    );
}

// ============================================================
// MOVER CAMARA
// ============================================================
//
// Esto mueve:
//
// position
// target
//
// al mismo tiempo.
//
// Sirve para desplazar toda la camara sin cambiar
// la direccion hacia donde esta mirando.
// ============================================================

inline void moveCamera(
    Camera& camera,
    Vec3 movement
) {
    camera.position =
        add(
            camera.position,
            movement
        );

    camera.target =
        add(
            camera.target,
            movement
        );
}

// ============================================================
// MOVER HACIA ADELANTE / ATRAS
// ============================================================

inline void moveCameraForward(
    Camera& camera,
    float amount
) {
    Vec3 forward =
        getCameraForward(
            camera
        );

    Vec3 movement =
        multiply(
            forward,
            amount
        );

    moveCamera(
        camera,
        movement
    );
}

// ============================================================
// MOVER A LOS LADOS
// ============================================================

inline void moveCameraRight(
    Camera& camera,
    float amount
) {
    Vec3 right =
        getCameraRight(
            camera
        );

    Vec3 movement =
        multiply(
            right,
            amount
        );

    moveCamera(
        camera,
        movement
    );
}

// ============================================================
// MOVER ARRIBA / ABAJO
// ============================================================

inline void moveCameraUp(
    Camera& camera,
    float amount
) {
    Vec3 movement =
        multiply(
            camera.up,
            amount
        );

    moveCamera(
        camera,
        movement
    );
}

// ============================================================
// CAMARA QUE SIGUE A UN OBJETO
// ============================================================
//
// Esta sera especialmente importante para nuestra nave.
//
// objectPosition:
// posicion de la nave.
//
// forward:
// direccion hacia donde apunta la nave.
//
// distance:
// distancia de la camara detras de la nave.
//
// height:
// altura de la camara sobre la nave.
// ============================================================

inline void followObject(
    Camera& camera,
    Vec3 objectPosition,
    Vec3 forward,
    float distance,
    float height
) {
    Vec3 normalizedForward =
        normalize(
            forward
        );

    Vec3 backwards =
        multiply(
            normalizedForward,
            -distance
        );

    Vec3 verticalOffset {
        0.0f,
        0.0f,
        height
    };

    camera.position =
        add(
            add(
                objectPosition,
                backwards
            ),
            verticalOffset
        );

    camera.target =
        add(
            objectPosition,
            multiply(
                normalizedForward,
                2.0f
            )
        );

    camera.up = {
        0.0f,
        0.0f,
        1.0f
    };
}

// ============================================================
// CAMARA ORBITAL ALREDEDOR DE UN PUNTO
// ============================================================
//
// Nos servira despues para el modo cercano a planeta.
//
// angle:
// angulo alrededor del objetivo.
//
// radius:
// distancia horizontal.
//
// height:
// altura de la camara.
// ============================================================

inline void setOrbitCamera(
    Camera& camera,
    Vec3 target,
    float angle,
    float radius,
    float height
) {
    camera.position.x =
        target.x
        +
        std::cos(angle)
        *
        radius;

    camera.position.y =
        target.y
        +
        std::sin(angle)
        *
        radius;

    camera.position.z =
        target.z
        +
        height;

    camera.target =
        target;

    camera.up = {
        0.0f,
        0.0f,
        1.0f
    };
}