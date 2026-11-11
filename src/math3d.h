#pragma once

#include <cmath>

// ============================================================
// CONSTANTES
// ============================================================

constexpr float PI = 3.1415926535f;

// ============================================================
// VEC3
// ============================================================

struct Vec3 {
    float x;
    float y;
    float z;
};

inline Vec3 add(
    Vec3 a,
    Vec3 b
) {
    return {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };
}

inline Vec3 subtract(
    Vec3 a,
    Vec3 b
) {
    return {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

inline Vec3 multiply(
    Vec3 v,
    float scalar
) {
    return {
        v.x * scalar,
        v.y * scalar,
        v.z * scalar
    };
}

inline float dot(
    Vec3 a,
    Vec3 b
) {
    return
        a.x * b.x
        +
        a.y * b.y
        +
        a.z * b.z;
}

inline Vec3 cross(
    Vec3 a,
    Vec3 b
) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

inline float length(
    Vec3 v
) {
    return std::sqrt(
        v.x * v.x
        +
        v.y * v.y
        +
        v.z * v.z
    );
}

inline Vec3 normalize(
    Vec3 v
) {
    float l =
        length(v);

    if (
        l <
        0.000001f
    ) {
        return {
            0.0f,
            0.0f,
            0.0f
        };
    }

    return {
        v.x / l,
        v.y / l,
        v.z / l
    };
}

// ============================================================
// VEC4
// ============================================================

struct Vec4 {
    float x;
    float y;
    float z;
    float w;
};

// ============================================================
// MATRIZ 4x4
// ============================================================

struct Mat4 {
    float m[4][4] = {};

    static Mat4 identity() {
        Mat4 result;

        result.m[0][0] = 1.0f;
        result.m[1][1] = 1.0f;
        result.m[2][2] = 1.0f;
        result.m[3][3] = 1.0f;

        return result;
    }
};

// ============================================================
// MATRIZ * MATRIZ
// ============================================================

inline Mat4 operator*(
    const Mat4& a,
    const Mat4& b
) {
    Mat4 result;

    for (
        int row = 0;
        row < 4;
        row++
    ) {
        for (
            int col = 0;
            col < 4;
            col++
        ) {
            result.m[row][col] =
                a.m[row][0] * b.m[0][col]
                +
                a.m[row][1] * b.m[1][col]
                +
                a.m[row][2] * b.m[2][col]
                +
                a.m[row][3] * b.m[3][col];
        }
    }

    return result;
}

// ============================================================
// MATRIZ * VECTOR
// ============================================================

inline Vec4 operator*(
    const Mat4& matrix,
    const Vec4& vector
) {
    return {
        matrix.m[0][0] * vector.x
            +
            matrix.m[0][1] * vector.y
            +
            matrix.m[0][2] * vector.z
            +
            matrix.m[0][3] * vector.w,

        matrix.m[1][0] * vector.x
            +
            matrix.m[1][1] * vector.y
            +
            matrix.m[1][2] * vector.z
            +
            matrix.m[1][3] * vector.w,

        matrix.m[2][0] * vector.x
            +
            matrix.m[2][1] * vector.y
            +
            matrix.m[2][2] * vector.z
            +
            matrix.m[2][3] * vector.w,

        matrix.m[3][0] * vector.x
            +
            matrix.m[3][1] * vector.y
            +
            matrix.m[3][2] * vector.z
            +
            matrix.m[3][3] * vector.w
    };
}

// ============================================================
// TRASLACION
// ============================================================

inline Mat4 makeTranslation(
    float x,
    float y,
    float z
) {
    Mat4 result =
        Mat4::identity();

    result.m[0][3] = x;
    result.m[1][3] = y;
    result.m[2][3] = z;

    return result;
}

// ============================================================
// ESCALA
// ============================================================

inline Mat4 makeScale(
    float x,
    float y,
    float z
) {
    Mat4 result =
        Mat4::identity();

    result.m[0][0] = x;
    result.m[1][1] = y;
    result.m[2][2] = z;

    return result;
}

// ============================================================
// ROTACION X
// ============================================================

inline Mat4 makeRotationX(
    float angle
) {
    Mat4 result =
        Mat4::identity();

    float c =
        std::cos(angle);

    float s =
        std::sin(angle);

    result.m[1][1] = c;
    result.m[1][2] = -s;

    result.m[2][1] = s;
    result.m[2][2] = c;

    return result;
}

// ============================================================
// ROTACION Y
// ============================================================

inline Mat4 makeRotationY(
    float angle
) {
    Mat4 result =
        Mat4::identity();

    float c =
        std::cos(angle);

    float s =
        std::sin(angle);

    result.m[0][0] = c;
    result.m[0][2] = s;

    result.m[2][0] = -s;
    result.m[2][2] = c;

    return result;
}

// ============================================================
// ROTACION Z
// ============================================================

inline Mat4 makeRotationZ(
    float angle
) {
    Mat4 result =
        Mat4::identity();

    float c =
        std::cos(angle);

    float s =
        std::sin(angle);

    result.m[0][0] = c;
    result.m[0][1] = -s;

    result.m[1][0] = s;
    result.m[1][1] = c;

    return result;
}

// ============================================================
// PROYECCION EN PERSPECTIVA
// ============================================================

inline Mat4 makePerspective(
    float fovRadians,
    float aspect,
    float nearPlane,
    float farPlane
) {
    Mat4 result;

    float f =
        1.0f
        /
        std::tan(
            fovRadians
            *
            0.5f
        );

    result.m[0][0] =
        f
        /
        aspect;

    result.m[1][1] =
        f;

    result.m[2][2] =
        (
            farPlane
            +
            nearPlane
        )
        /
        (
            nearPlane
            -
            farPlane
        );

    result.m[2][3] =
        (
            2.0f
            *
            farPlane
            *
            nearPlane
        )
        /
        (
            nearPlane
            -
            farPlane
        );

    result.m[3][2] =
        -1.0f;

    return result;
}

// ============================================================
// VIEW MATRIX / LOOK AT
// ============================================================

inline Mat4 makeLookAt(
    Vec3 eye,
    Vec3 target,
    Vec3 up
) {
    Vec3 forward =
        normalize(
            subtract(
                target,
                eye
            )
        );

    Vec3 right =
        normalize(
            cross(
                forward,
                up
            )
        );

    Vec3 cameraUp =
        cross(
            right,
            forward
        );

    Mat4 result =
        Mat4::identity();

    result.m[0][0] =
        right.x;

    result.m[0][1] =
        right.y;

    result.m[0][2] =
        right.z;

    result.m[0][3] =
        -dot(
            right,
            eye
        );

    result.m[1][0] =
        cameraUp.x;

    result.m[1][1] =
        cameraUp.y;

    result.m[1][2] =
        cameraUp.z;

    result.m[1][3] =
        -dot(
            cameraUp,
            eye
        );

    result.m[2][0] =
        -forward.x;

    result.m[2][1] =
        -forward.y;

    result.m[2][2] =
        -forward.z;

    result.m[2][3] =
        dot(
            forward,
            eye
        );

    return result;
}

// ============================================================
// GRADOS -> RADIANES
// ============================================================

inline float radians(
    float degrees
) {
    return
        degrees
        *
        PI
        /
        180.0f;
}