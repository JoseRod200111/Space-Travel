#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "camera.h"
#include "color.h"
#include "framebuffer.h"
#include "geometry.h"
#include "math3d.h"
#include "mesh.h"
#include "obj_loader.h"
#include "scene.h"
#include "shaders.h"
#include "triangle.h"

// ============================================================
// SDL
// ============================================================

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

// ============================================================
// MESHES
// ============================================================

Mesh spaceshipMesh;
Mesh sunMesh;
Mesh planetMesh;
Mesh moonMesh;
Mesh ringMesh;
Mesh cubeMesh;

// ============================================================
// OBJETOS
// ============================================================

SceneObject spaceship;

SceneObject sun;

SceneObject rockyPlanet;
SceneObject greenPlanet;

SceneObject ringPlanet;
SceneObject rings;

SceneObject moon;

// Objetos sobre el planeta verde.
SceneObject surfaceBase;
SceneObject surfaceBuilding;
SceneObject surfaceTower;
SceneObject surfaceBeacon;

// ============================================================
// CAMARA
// ============================================================

Camera camera;

// ============================================================
// ESTADO DE LA NAVE
// ============================================================

float shipHeading = 0.0f;

float shipSpeed = 0.0f;
float verticalSpeed = 0.0f;
float turnSpeed = 0.0f;

// ============================================================
// WARP
// ============================================================

float warpAmount = 0.0f;

// ============================================================
// MOVIMIENTO
// ============================================================

constexpr float MAX_FORWARD_SPEED = 6.0f;
constexpr float MAX_REVERSE_SPEED = -2.0f;

constexpr float ACCELERATION = 3.5f;
constexpr float BRAKE_ACCELERATION = 5.0f;

constexpr float NATURAL_DRAG = 0.8f;

constexpr float MAX_TURN_SPEED = 1.5f;
constexpr float TURN_ACCELERATION = 3.0f;
constexpr float TURN_DRAG = 4.0f;

constexpr float MAX_VERTICAL_SPEED = 3.5f;
constexpr float VERTICAL_ACCELERATION = 5.0f;
constexpr float VERTICAL_DRAG = 5.0f;

// ============================================================
// WARP
// ============================================================

constexpr float WARP_MAX_SPEED = 24.0f;
constexpr float WARP_ACCELERATION = 14.0f;

constexpr float WARP_CHARGE_SPEED = 1.7f;
constexpr float WARP_RELEASE_SPEED = 2.3f;

constexpr float WARP_EXIT_DECELERATION = 10.0f;

constexpr float NORMAL_FOV = 68.0f;
constexpr float WARP_FOV = 80.0f;

// ============================================================
// SUPERFICIE
// ============================================================

constexpr float SURFACE_OBJECT_VISIBILITY_DISTANCE = 10.0f;

// ============================================================
// UTILIDADES
// ============================================================

float moveToward(
    float current,
    float target,
    float amount
) {
    if (current < target) {
        current += amount;

        if (current > target) {
            current = target;
        }
    }
    else if (current > target) {
        current -= amount;

        if (current < target) {
            current = target;
        }
    }

    return current;
}

Vec3 rotateAroundZ(
    const Vec3& point,
    float angle
) {
    float c =
        std::cos(angle);

    float s =
        std::sin(angle);

    return {
        point.x * c - point.y * s,
        point.x * s + point.y * c,
        point.z
    };
}

// ============================================================
// SDL
// ============================================================

bool initSDL() {
    if (
        SDL_Init(SDL_INIT_VIDEO)
        !=
        0
    ) {
        std::cout
            << "ERROR SDL: "
            << SDL_GetError()
            << std::endl;

        return false;
    }

    window =
        SDL_CreateWindow(
            "Proyecto 3 - Space Travel",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN
        );

    if (
        window
        ==
        nullptr
    ) {
        std::cout
            << "ERROR creando ventana: "
            << SDL_GetError()
            << std::endl;

        return false;
    }

    renderer =
        SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED
        );

    if (
        renderer
        ==
        nullptr
    ) {
        std::cout
            << "ERROR creando renderer: "
            << SDL_GetError()
            << std::endl;

        return false;
    }

    return true;
}

// ============================================================
// NORMALIZAR OBJ
// ============================================================

void normalizeMesh(
    Mesh& mesh,
    float targetSize
) {
    if (
        mesh.vertices.empty()
    ) {
        return;
    }

    MeshBounds bounds =
        calculateMeshBounds(
            mesh
        );

    if (
        bounds.largestSize
        <
        0.0001f
    ) {
        return;
    }

    float scale =
        targetSize
        /
        bounds.largestSize;

    for (
        Vertex& vertex :
        mesh.vertices
    ) {
        vertex.position =
            subtract(
                vertex.position,
                bounds.center
            );

        vertex.position =
            multiply(
                vertex.position,
                scale
            );
    }
}

// ============================================================
// SHADERS
// ============================================================

Color runShader(
    ShaderType shaderType,
    const FragmentInput& input
) {
    switch (
        shaderType
    ) {
        case ShaderType::Spaceship:
            return spaceshipShader(input);

        case ShaderType::Sun:
            return sunShader(input);

        case ShaderType::RockyPlanet:
            return rockyPlanetShader(input);

        case ShaderType::GreenPlanet:
            return greenPlanetShader(input);

        case ShaderType::Rings:
            return ringShader(input);

        case ShaderType::FlatColor:
        default:
            return applyIntensity(
                input.baseColor,
                basicLighting(
                    input.normal
                )
            );
    }
}

// ============================================================
// CLIP -> SCREEN
// ============================================================

bool clipToScreen(
    const Vec4& clip,
    ScreenVertex& result
) {
    if (
        clip.w
        <=
        0.001f
    ) {
        return false;
    }

    float ndcX =
        clip.x
        /
        clip.w;

    float ndcY =
        clip.y
        /
        clip.w;

    float ndcZ =
        clip.z
        /
        clip.w;

    if (
        ndcZ < -1.0f
        ||
        ndcZ > 1.0f
    ) {
        return false;
    }

    result.x =
        (
            ndcX * 0.5f
            +
            0.5f
        )
        *
        static_cast<float>(
            SCREEN_WIDTH
        );

    result.y =
        (
            1.0f
            -
            (
                ndcY * 0.5f
                +
                0.5f
            )
        )
        *
        static_cast<float>(
            SCREEN_HEIGHT
        );

    result.depth =
        ndcZ * 0.5f
        +
        0.5f;

    return true;
}

// ============================================================
// RENDER OBJETO
// ============================================================

void renderObject(
    const SceneObject& object,
    const Mat4& view,
    const Mat4& projection,
    float elapsedTime
) {
    if (
        !object.visible
        ||
        object.mesh == nullptr
    ) {
        return;
    }

    const Mesh& mesh =
        *object.mesh;

    if (
        mesh.vertices.empty()
        ||
        mesh.faces.empty()
    ) {
        return;
    }

    Mat4 model =
        getModelMatrix(
            object
        );

    Mat4 mvp =
        projection
        *
        view
        *
        model;

    std::vector<Vec3> worldVertices(
        mesh.vertices.size()
    );

    std::vector<Vec4> clipVertices(
        mesh.vertices.size()
    );

    // ========================================================
    // TRANSFORMAR VERTICES
    // ========================================================

    for (
        std::size_t i = 0;
        i < mesh.vertices.size();
        i++
    ) {
        const Vec3& position =
            mesh.vertices[i].position;

        Vec4 local {
            position.x,
            position.y,
            position.z,
            1.0f
        };

        Vec4 world =
            model * local;

        worldVertices[i] = {
            world.x,
            world.y,
            world.z
        };

        clipVertices[i] =
            mvp * local;
    }

    // ========================================================
    // TRIANGULOS
    // ========================================================

    for (
        const Face& face :
        mesh.faces
    ) {
        if (
            face.a < 0
            ||
            face.b < 0
            ||
            face.c < 0
        ) {
            continue;
        }

        if (
            face.a >=
                static_cast<int>(
                    mesh.vertices.size()
                )
            ||
            face.b >=
                static_cast<int>(
                    mesh.vertices.size()
                )
            ||
            face.c >=
                static_cast<int>(
                    mesh.vertices.size()
                )
        ) {
            continue;
        }

        Vec3 worldA =
            worldVertices[
                face.a
            ];

        Vec3 worldB =
            worldVertices[
                face.b
            ];

        Vec3 worldC =
            worldVertices[
                face.c
            ];

        Vec3 edge1 =
            subtract(
                worldB,
                worldA
            );

        Vec3 edge2 =
            subtract(
                worldC,
                worldA
            );

        Vec3 normal =
            normalize(
                cross(
                    edge1,
                    edge2
                )
            );

        ScreenVertex screenA;
        ScreenVertex screenB;
        ScreenVertex screenC;

        if (
            !clipToScreen(
                clipVertices[
                    face.a
                ],
                screenA
            )
            ||
            !clipToScreen(
                clipVertices[
                    face.b
                ],
                screenB
            )
            ||
            !clipToScreen(
                clipVertices[
                    face.c
                ],
                screenC
            )
        ) {
            continue;
        }

        auto fragmentShader =
            [
                worldA,
                worldB,
                worldC,
                normal,
                elapsedTime,
                &object
            ](
                float w0,
                float w1,
                float w2
            ) -> Color {

                Vec3 fragmentPosition {
                    worldA.x * w0
                        +
                        worldB.x * w1
                        +
                        worldC.x * w2,

                    worldA.y * w0
                        +
                        worldB.y * w1
                        +
                        worldC.y * w2,

                    worldA.z * w0
                        +
                        worldB.z * w1
                        +
                        worldC.z * w2
                };

                FragmentInput input;

                input.worldPosition =
                    fragmentPosition;

                input.normal =
                    normal;

                input.time =
                    elapsedTime;

                input.baseColor =
                    object.baseColor;

                return runShader(
                    object.shaderType,
                    input
                );
            };

        triangle(
            screenA,
            screenB,
            screenC,
            fragmentShader
        );
    }
}

// ============================================================
// WARP STAR
// ============================================================

void drawWarpStar(
    int screenX,
    int screenY,
    Color starColor,
    float currentWarp,
    bool large
) {
    point(
        screenX,
        screenY,
        starColor
    );

    if (
        large
    ) {
        point(
            screenX + 1,
            screenY,
            starColor
        );

        point(
            screenX - 1,
            screenY,
            starColor
        );

        point(
            screenX,
            screenY + 1,
            starColor
        );

        point(
            screenX,
            screenY - 1,
            starColor
        );
    }

    if (
        currentWarp
        <=
        0.02f
    ) {
        return;
    }

    float centerX =
        SCREEN_WIDTH
        *
        0.5f;

    float centerY =
        SCREEN_HEIGHT
        *
        0.5f;

    float dx =
        static_cast<float>(
            screenX
        )
        -
        centerX;

    float dy =
        static_cast<float>(
            screenY
        )
        -
        centerY;

    float distance =
        std::sqrt(
            dx * dx
            +
            dy * dy
        );

    if (
        distance
        <
        1.0f
    ) {
        return;
    }

    float directionX =
        dx
        /
        distance;

    float directionY =
        dy
        /
        distance;

    int trailLength =
        static_cast<int>(
            2.0f
            +
            currentWarp
            *
            20.0f
        );

    for (
        int step = 1;
        step <= trailLength;
        step++
    ) {
        float fade =
            1.0f
            -
            static_cast<float>(
                step
            )
            /
            static_cast<float>(
                trailLength
                +
                1
            );

        float intensity =
            (
                0.25f
                +
                0.75f
                *
                fade
            )
            *
            currentWarp;

        unsigned char r =
            static_cast<unsigned char>(
                std::min(
                    255.0f,
                    100.0f
                    +
                    155.0f
                    *
                    intensity
                )
            );

        unsigned char g =
            static_cast<unsigned char>(
                std::min(
                    255.0f,
                    150.0f
                    +
                    105.0f
                    *
                    intensity
                )
            );

        unsigned char b =
            255;

        int trailX =
            static_cast<int>(
                static_cast<float>(
                    screenX
                )
                -
                directionX
                *
                static_cast<float>(
                    step
                )
            );

        int trailY =
            static_cast<int>(
                static_cast<float>(
                    screenY
                )
                -
                directionY
                *
                static_cast<float>(
                    step
                )
            );

        point(
            trailX,
            trailY,
            Color(
                r,
                g,
                b,
                255
            )
        );
    }
}

// ============================================================
// SKYBOX
// ============================================================

void drawSkyboxStars(
    const Camera& activeCamera,
    float currentWarp
) {
    Vec3 forward =
        getCameraForward(
            activeCamera
        );

    Vec3 right =
        getCameraRight(
            activeCamera
        );

    Vec3 up =
        getCameraUp(
            activeCamera
        );

    float aspect =
        static_cast<float>(
            SCREEN_WIDTH
        )
        /
        static_cast<float>(
            SCREEN_HEIGHT
        );

    float tanHalfFov =
        std::tan(
            radians(
                activeCamera.fovDegrees
            )
            *
            0.5f
        );

    constexpr int STAR_COUNT =
        950;

    constexpr float GOLDEN_ANGLE =
        2.39996323f;

    // ========================================================
    // ESTRELLAS
    // ========================================================

    for (
        int i = 0;
        i < STAR_COUNT;
        i++
    ) {
        float t =
            (
                static_cast<float>(i)
                +
                0.5f
            )
            /
            static_cast<float>(
                STAR_COUNT
            );

        float z =
            1.0f
            -
            2.0f
            *
            t;

        float horizontal =
            std::sqrt(
                std::max(
                    0.0f,
                    1.0f
                    -
                    z * z
                )
            );

        float angle =
            static_cast<float>(
                i
            )
            *
            GOLDEN_ANGLE;

        Vec3 starDirection {
            horizontal
                *
                std::cos(
                    angle
                ),

            horizontal
                *
                std::sin(
                    angle
                ),

            z
        };

        float cameraX =
            dot(
                starDirection,
                right
            );

        float cameraY =
            dot(
                starDirection,
                up
            );

        float cameraZ =
            dot(
                starDirection,
                forward
            );

        if (
            cameraZ
            <=
            0.01f
        ) {
            continue;
        }

        float ndcX =
            cameraX
            /
            (
                cameraZ
                *
                tanHalfFov
                *
                aspect
            );

        float ndcY =
            cameraY
            /
            (
                cameraZ
                *
                tanHalfFov
            );

        if (
            ndcX < -1.0f
            ||
            ndcX > 1.0f
            ||
            ndcY < -1.0f
            ||
            ndcY > 1.0f
        ) {
            continue;
        }

        int screenX =
            static_cast<int>(
                (
                    ndcX * 0.5f
                    +
                    0.5f
                )
                *
                SCREEN_WIDTH
            );

        int screenY =
            static_cast<int>(
                (
                    1.0f
                    -
                    (
                        ndcY * 0.5f
                        +
                        0.5f
                    )
                )
                *
                SCREEN_HEIGHT
            );

        int brightnessValue =
            135
            +
            (
                i
                *
                73
            )
            %
            121;

        unsigned char brightness =
            static_cast<unsigned char>(
                brightnessValue
            );

        Color starColor;

        if (
            i % 13
            ==
            0
        ) {
            starColor =
                Color(
                    static_cast<unsigned char>(
                        brightness
                        *
                        0.78f
                    ),

                    static_cast<unsigned char>(
                        brightness
                        *
                        0.88f
                    ),

                    brightness,
                    255
                );
        }
        else if (
            i % 19
            ==
            0
        ) {
            starColor =
                Color(
                    brightness,

                    static_cast<unsigned char>(
                        brightness
                        *
                        0.86f
                    ),

                    static_cast<unsigned char>(
                        brightness
                        *
                        0.70f
                    ),

                    255
                );
        }
        else {
            starColor =
                Color(
                    brightness,
                    brightness,
                    brightness,
                    255
                );
        }

        drawWarpStar(
            screenX,
            screenY,
            starColor,
            currentWarp,
            i % 47 == 0
        );
    }

    // ========================================================
    // BANDA ESTELAR
    // ========================================================

    constexpr int BAND_STAR_COUNT =
        280;

    for (
        int i = 0;
        i < BAND_STAR_COUNT;
        i++
    ) {
        float angle =
            static_cast<float>(
                i
            )
            /
            static_cast<float>(
                BAND_STAR_COUNT
            )
            *
            2.0f
            *
            PI;

        float verticalVariation =
            std::sin(
                static_cast<float>(
                    i
                )
                *
                4.73f
            )
            *
            0.10f;

        Vec3 starDirection =
            normalize({
                std::cos(
                    angle
                ),

                std::sin(
                    angle
                ),

                verticalVariation
            });

        float cameraX =
            dot(
                starDirection,
                right
            );

        float cameraY =
            dot(
                starDirection,
                up
            );

        float cameraZ =
            dot(
                starDirection,
                forward
            );

        if (
            cameraZ
            <=
            0.01f
        ) {
            continue;
        }

        float ndcX =
            cameraX
            /
            (
                cameraZ
                *
                tanHalfFov
                *
                aspect
            );

        float ndcY =
            cameraY
            /
            (
                cameraZ
                *
                tanHalfFov
            );

        if (
            ndcX < -1.0f
            ||
            ndcX > 1.0f
            ||
            ndcY < -1.0f
            ||
            ndcY > 1.0f
        ) {
            continue;
        }

        int screenX =
            static_cast<int>(
                (
                    ndcX * 0.5f
                    +
                    0.5f
                )
                *
                SCREEN_WIDTH
            );

        int screenY =
            static_cast<int>(
                (
                    1.0f
                    -
                    (
                        ndcY * 0.5f
                        +
                        0.5f
                    )
                )
                *
                SCREEN_HEIGHT
            );

        unsigned char brightness =
            static_cast<unsigned char>(
                100
                +
                (
                    i * 31
                )
                %
                100
            );

        drawWarpStar(
            screenX,
            screenY,

            Color(
                brightness,
                brightness,
                brightness,
                255
            ),

            currentWarp,
            false
        );
    }
}

// ============================================================
// DIRECCION NAVE
// ============================================================

Vec3 getShipForward() {
    return normalize({
        std::sin(
            shipHeading
        ),

        std::cos(
            shipHeading
        ),

        0.0f
    });
}

// ============================================================
// COLISIONES
// ============================================================

bool resolveSpaceshipCollision(
    const SceneObject& body
) {
    float shipRadius =
        spaceship.collisionRadius;

    if (
        !pointInsideObject(
            spaceship.transform.position,
            body,
            shipRadius
        )
    ) {
        return false;
    }

    spaceship.transform.position =
        pushOutsideObject(
            spaceship.transform.position,
            body,
            shipRadius
        );

    return true;
}

void resolveCameraCollision(
    const SceneObject& body
) {
    constexpr float CAMERA_MARGIN =
        0.35f;

    if (
        pointInsideObject(
            camera.position,
            body,
            CAMERA_MARGIN
        )
    ) {
        camera.position =
            pushOutsideObject(
                camera.position,
                body,
                CAMERA_MARGIN
            );
    }
}

// ============================================================
// CREAR ESCENA
// ============================================================

bool createScene() {
    // ========================================================
    // NAVE OBJ
    // ========================================================

    if (
        !loadOBJ(
            "models/nave.obj",
            spaceshipMesh
        )
    ) {
        return false;
    }

    normalizeMesh(
        spaceshipMesh,
        2.6f
    );

    // ========================================================
    // GEOMETRIA PROCEDURAL
    // ========================================================

    sunMesh =
        createSphere(
            14,
            20,
            1.0f
        );

    planetMesh =
        createSphere(
            12,
            18,
            1.0f
        );

    moonMesh =
        createSphere(
            10,
            14,
            1.0f
        );

    ringMesh =
        createRing(
            40,
            1.35f,
            2.15f,
            0.07f
        );

    cubeMesh =
        createCube(
            1.0f
        );

    // ========================================================
    // SOL
    // ========================================================

    sun =
        createSceneObject(
            "Sol",
            &sunMesh,
            ShaderType::Sun,

            {
                0.0f,
                0.0f,
                0.0f
            },

            {
                2.3f,
                2.3f,
                2.3f
            }
        );

    configureRotation(
        sun,
        0.18f
    );

    configureCollision(
        sun,
        2.3f
    );

    // ========================================================
    // PLANETA ROCOSO
    // ========================================================

    rockyPlanet =
        createSceneObject(
            "Planeta Rocoso",
            &planetMesh,
            ShaderType::RockyPlanet,

            {
                8.0f,
                0.0f,
                0.0f
            },

            {
                1.25f,
                1.25f,
                1.25f
            }
        );

    configureOrbit(
        rockyPlanet,
        8.0f,
        0.23f,
        0.25f,
        0.0f
    );

    configureRotation(
        rockyPlanet,
        0.75f
    );

    configureCollision(
        rockyPlanet,
        1.25f
    );

    // ========================================================
    // PLANETA VERDE
    // ========================================================

    greenPlanet =
        createSceneObject(
            "Planeta Verde",
            &planetMesh,
            ShaderType::GreenPlanet,

            {
                13.0f,
                0.0f,
                0.0f
            },

            {
                1.55f,
                1.55f,
                1.55f
            }
        );

    configureOrbit(
        greenPlanet,
        13.0f,
        0.14f,
        2.0f,
        0.25f
    );

    configureRotation(
        greenPlanet,
        -0.48f
    );

    configureCollision(
        greenPlanet,
        1.55f
    );

    // ========================================================
    // PLANETA ANILLADO
    // ========================================================

    ringPlanet =
        createSceneObject(
            "Planeta Anillado",
            &planetMesh,
            ShaderType::RockyPlanet,

            {
                18.0f,
                0.0f,
                0.0f
            },

            {
                1.7f,
                1.7f,
                1.7f
            }
        );

    configureOrbit(
        ringPlanet,
        18.0f,
        0.085f,
        4.1f,
        -0.2f
    );

    configureRotation(
        ringPlanet,
        0.38f
    );

    configureCollision(
        ringPlanet,
        1.7f
    );

    // ========================================================
    // ANILLOS
    // ========================================================

    rings =
        createSceneObject(
            "Anillos",
            &ringMesh,
            ShaderType::Rings,

            ringPlanet.transform.position,

            {
                1.15f,
                1.15f,
                1.15f
            }
        );

    rings.transform.rotation.x =
        radians(
            23.0f
        );

    // ========================================================
    // LUNA
    // ========================================================

    moon =
        createSceneObject(
            "Luna",
            &moonMesh,
            ShaderType::RockyPlanet,

            {
                0.0f,
                0.0f,
                0.0f
            },

            {
                0.42f,
                0.42f,
                0.42f
            }
        );

    configureOrbit(
        moon,
        2.5f,
        0.95f,
        1.2f,
        0.15f
    );

    configureRotation(
        moon,
        0.8f
    );

    configureCollision(
        moon,
        0.42f
    );

    // ========================================================
    // BASE PLANETARIA
    // ========================================================

    surfaceBase =
        createSceneObject(
            "Plataforma",
            &cubeMesh,
            ShaderType::FlatColor,

            {
                0.0f,
                0.0f,
                0.0f
            },

            {
                0.95f,
                0.70f,
                0.10f
            }
        );

    surfaceBase.baseColor =
        Color(
            115,
            125,
            145,
            255
        );

    surfaceBuilding =
        createSceneObject(
            "Edificio",
            &cubeMesh,
            ShaderType::FlatColor,

            {
                0.0f,
                0.0f,
                0.0f
            },

            {
                0.38f,
                0.32f,
                0.34f
            }
        );

    surfaceBuilding.baseColor =
        Color(
            65,
            120,
            170,
            255
        );

    surfaceTower =
        createSceneObject(
            "Torre",
            &cubeMesh,
            ShaderType::FlatColor,

            {
                0.0f,
                0.0f,
                0.0f
            },

            {
                0.16f,
                0.16f,
                0.85f
            }
        );

    surfaceTower.baseColor =
        Color(
            175,
            185,
            200,
            255
        );

    surfaceBeacon =
        createSceneObject(
            "Baliza",
            &cubeMesh,
            ShaderType::FlatColor,

            {
                0.0f,
                0.0f,
                0.0f
            },

            {
                0.12f,
                0.12f,
                0.12f
            }
        );

    surfaceBeacon.baseColor =
        Color(
            255,
            75,
            45,
            255
        );

    surfaceBase.visible =
        false;

    surfaceBuilding.visible =
        false;

    surfaceTower.visible =
        false;

    surfaceBeacon.visible =
        false;

    // ========================================================
    // NAVE
    // ========================================================

    spaceship =
        createSceneObject(
            "Nave",
            &spaceshipMesh,
            ShaderType::Spaceship,

            {
                0.0f,
                -12.0f,
                0.0f
            },

            {
                1.0f,
                1.0f,
                1.0f
            }
        );

    shipHeading =
        0.0f;

    spaceship.transform.rotation.z =
        radians(
            180.0f
        );

    configureCollision(
        spaceship,
        0.9f
    );

    // ========================================================
    // CAMARA
    // ========================================================

    camera.fovDegrees =
        NORMAL_FOV;

    camera.nearPlane =
        0.1f;

    camera.farPlane =
        600.0f;

    followObject(
        camera,
        spaceship.transform.position,
        getShipForward(),
        5.8f,
        2.7f
    );

    return true;
}

// ============================================================
// BASE PLANETARIA
// ============================================================

void updateSurfaceStructures() {
    float angle =
        greenPlanet.transform.rotation.z;

    Vec3 baseLocal {
        0.0f,
        0.0f,
        1.60f
    };

    Vec3 buildingLocal {
        -0.25f,
        -0.10f,
        1.78f
    };

    Vec3 towerLocal {
        0.27f,
        0.08f,
        2.02f
    };

    Vec3 beaconLocal {
        0.27f,
        0.08f,
        2.50f
    };

    Vec3 rotatedBase =
        rotateAroundZ(
            baseLocal,
            angle
        );

    Vec3 rotatedBuilding =
        rotateAroundZ(
            buildingLocal,
            angle
        );

    Vec3 rotatedTower =
        rotateAroundZ(
            towerLocal,
            angle
        );

    Vec3 rotatedBeacon =
        rotateAroundZ(
            beaconLocal,
            angle
        );

    surfaceBase.transform.position =
        add(
            greenPlanet.transform.position,
            rotatedBase
        );

    surfaceBuilding.transform.position =
        add(
            greenPlanet.transform.position,
            rotatedBuilding
        );

    surfaceTower.transform.position =
        add(
            greenPlanet.transform.position,
            rotatedTower
        );

    surfaceBeacon.transform.position =
        add(
            greenPlanet.transform.position,
            rotatedBeacon
        );

    surfaceBase.transform.rotation.z =
        angle;

    surfaceBuilding.transform.rotation.z =
        angle;

    surfaceTower.transform.rotation.z =
        angle;

    surfaceBeacon.transform.rotation.z =
        angle;

    float distanceToPlanet =
        distanceBetween(
            spaceship.transform.position,
            greenPlanet.transform.position
        );

    bool showStructures =
        distanceToPlanet
        <
        SURFACE_OBJECT_VISIBILITY_DISTANCE;

    surfaceBase.visible =
        showStructures;

    surfaceBuilding.visible =
        showStructures;

    surfaceTower.visible =
        showStructures;

    surfaceBeacon.visible =
        showStructures;
}

// ============================================================
// ACTUALIZAR NAVE
// ============================================================

void updateSpaceship(
    float deltaTime
) {
    const Uint8* keyboard =
        SDL_GetKeyboardState(
            nullptr
        );

    bool warpPressed =
        keyboard[
            SDL_SCANCODE_LSHIFT
        ]
        ||
        keyboard[
            SDL_SCANCODE_RSHIFT
        ];

    if (
        keyboard[
            SDL_SCANCODE_S
        ]
    ) {
        warpPressed =
            false;
    }

    // ========================================================
    // WARP
    // ========================================================

    if (
        warpPressed
    ) {
        warpAmount =
            moveToward(
                warpAmount,
                1.0f,
                WARP_CHARGE_SPEED
                    *
                    deltaTime
            );
    }
    else {
        warpAmount =
            moveToward(
                warpAmount,
                0.0f,
                WARP_RELEASE_SPEED
                    *
                    deltaTime
            );
    }

    // ========================================================
    // ACELERACION
    // ========================================================

    if (
        keyboard[
            SDL_SCANCODE_W
        ]
    ) {
        shipSpeed +=
            ACCELERATION
            *
            deltaTime;
    }

    if (
        keyboard[
            SDL_SCANCODE_S
        ]
    ) {
        shipSpeed -=
            BRAKE_ACCELERATION
            *
            deltaTime;
    }

    if (
        warpPressed
    ) {
        float warpTarget =
            MAX_FORWARD_SPEED
            +
            (
                WARP_MAX_SPEED
                -
                MAX_FORWARD_SPEED
            )
            *
            warpAmount;

        shipSpeed =
            moveToward(
                shipSpeed,
                warpTarget,
                WARP_ACCELERATION
                    *
                    deltaTime
            );
    }

    if (
        !warpPressed
        &&
        shipSpeed
            >
            MAX_FORWARD_SPEED
    ) {
        shipSpeed =
            moveToward(
                shipSpeed,
                MAX_FORWARD_SPEED,
                WARP_EXIT_DECELERATION
                    *
                    deltaTime
            );
    }

    if (
        !keyboard[
            SDL_SCANCODE_W
        ]
        &&
        !keyboard[
            SDL_SCANCODE_S
        ]
        &&
        !warpPressed
        &&
        shipSpeed
            <=
            MAX_FORWARD_SPEED
    ) {
        shipSpeed =
            moveToward(
                shipSpeed,
                0.0f,
                NATURAL_DRAG
                    *
                    deltaTime
            );
    }

    shipSpeed =
        std::clamp(
            shipSpeed,
            MAX_REVERSE_SPEED,
            WARP_MAX_SPEED
        );

    // ========================================================
    // GIRO
    // ========================================================

    float desiredTurn =
        0.0f;

    if (
        keyboard[
            SDL_SCANCODE_A
        ]
    ) {
        desiredTurn =
            MAX_TURN_SPEED;
    }

    if (
        keyboard[
            SDL_SCANCODE_D
        ]
    ) {
        desiredTurn =
            -MAX_TURN_SPEED;
    }

    float warpTurnMultiplier =
        1.0f
        -
        warpAmount
        *
        0.65f;

    desiredTurn *=
        warpTurnMultiplier;

    if (
        desiredTurn
        !=
        0.0f
    ) {
        turnSpeed =
            moveToward(
                turnSpeed,
                desiredTurn,
                TURN_ACCELERATION
                    *
                    deltaTime
            );
    }
    else {
        turnSpeed =
            moveToward(
                turnSpeed,
                0.0f,
                TURN_DRAG
                    *
                    deltaTime
            );
    }

    shipHeading +=
        turnSpeed
        *
        deltaTime;

    // ========================================================
    // MOVIMIENTO VERTICAL
    // ========================================================

    float desiredVertical =
        0.0f;

    if (
        keyboard[
            SDL_SCANCODE_Q
        ]
    ) {
        desiredVertical =
            MAX_VERTICAL_SPEED;
    }

    if (
        keyboard[
            SDL_SCANCODE_E
        ]
    ) {
        desiredVertical =
            -MAX_VERTICAL_SPEED;
    }

    if (
        desiredVertical
        !=
        0.0f
    ) {
        verticalSpeed =
            moveToward(
                verticalSpeed,
                desiredVertical,
                VERTICAL_ACCELERATION
                    *
                    deltaTime
            );
    }
    else {
        verticalSpeed =
            moveToward(
                verticalSpeed,
                0.0f,
                VERTICAL_DRAG
                    *
                    deltaTime
            );
    }

    // ========================================================
    // MOVIMIENTO
    // ========================================================

    Vec3 forward =
        getShipForward();

    Vec3 movement =
        multiply(
            forward,
            shipSpeed
                *
                deltaTime
        );

    spaceship.transform.position =
        add(
            spaceship.transform.position,
            movement
        );

    spaceship.transform.position.z +=
        verticalSpeed
        *
        deltaTime;

    // ========================================================
    // COLISIONES
    // ========================================================

    bool collided =
        false;

    if (
        resolveSpaceshipCollision(
            sun
        )
    ) {
        collided = true;
    }

    if (
        resolveSpaceshipCollision(
            rockyPlanet
        )
    ) {
        collided = true;
    }

    if (
        resolveSpaceshipCollision(
            greenPlanet
        )
    ) {
        collided = true;
    }

    if (
        resolveSpaceshipCollision(
            ringPlanet
        )
    ) {
        collided = true;
    }

    if (
        resolveSpaceshipCollision(
            moon
        )
    ) {
        collided = true;
    }

    if (
        collided
    ) {
        shipSpeed =
            0.0f;

        verticalSpeed =
            0.0f;

        warpAmount =
            0.0f;
    }

    // ========================================================
    // ROTACION VISUAL DE NAVE
    // ========================================================

    spaceship.transform.rotation.z =
        radians(
            180.0f
        )
        -
        shipHeading;

    // ========================================================
    // FOV WARP
    // ========================================================

    camera.fovDegrees =
        NORMAL_FOV
        +
        (
            WARP_FOV
            -
            NORMAL_FOV
        )
        *
        warpAmount;

    // ========================================================
    // CAMARA
    // ========================================================

    followObject(
        camera,
        spaceship.transform.position,
        forward,
        5.8f,
        2.7f
    );

    resolveCameraCollision(
        sun
    );

    resolveCameraCollision(
        rockyPlanet
    );

    resolveCameraCollision(
        greenPlanet
    );

    resolveCameraCollision(
        ringPlanet
    );

    resolveCameraCollision(
        moon
    );
}

// ============================================================
// ACTUALIZAR ESCENA
// ============================================================

void updateScene(
    float elapsedTime,
    float deltaTime
) {
    // ========================================================
    // SOL
    // ========================================================

    updateRotation(
        sun,
        elapsedTime
    );

    // ========================================================
    // PLANETA ROCOSO
    //
    // Orbita Y rota explicitamente usando elapsedTime.
    // ========================================================

    updateOrbitalObject(
        rockyPlanet,
        sun.transform.position,
        elapsedTime
    );

    updateRotation(
        rockyPlanet,
        elapsedTime
    );

    // ========================================================
    // PLANETA VERDE
    //
    // Orbita Y rota explicitamente usando elapsedTime.
    // ========================================================

    updateOrbitalObject(
        greenPlanet,
        sun.transform.position,
        elapsedTime
    );

    updateRotation(
        greenPlanet,
        elapsedTime
    );

    // ========================================================
    // PLANETA ANILLADO
    //
    // Orbita Y rota explicitamente usando elapsedTime.
    // ========================================================

    updateOrbitalObject(
        ringPlanet,
        sun.transform.position,
        elapsedTime
    );

    updateRotation(
        ringPlanet,
        elapsedTime
    );

    // ========================================================
    // ANILLOS
    // ========================================================

    rings.transform.position =
        ringPlanet.transform.position;

    rings.transform.rotation.x =
        radians(
            23.0f
        );

    rings.transform.rotation.z =
        ringPlanet.transform.rotation.z;

    // ========================================================
    // LUNA
    //
    // Tambien orbita y rota segun elapsedTime.
    // ========================================================

    updateOrbitalObject(
        moon,
        greenPlanet.transform.position,
        elapsedTime
    );

    updateRotation(
        moon,
        elapsedTime
    );

    // ========================================================
    // NAVE
    // ========================================================

    updateSpaceship(
        deltaTime
    );

    // ========================================================
    // ESTRUCTURAS DEL PLANETA
    // ========================================================

    updateSurfaceStructures();
}

// ============================================================
// RENDER
// ============================================================

void renderFrame(
    float elapsedTime
) {
    // ========================================================
    // FONDO
    // ========================================================

    int blueBoost =
        static_cast<int>(
            10.0f
            *
            warpAmount
        );

    clear(
        Color(
            2,
            3,
            static_cast<unsigned char>(
                10
                +
                blueBoost
            ),
            255
        )
    );

    // ========================================================
    // SKYBOX
    // ========================================================

    drawSkyboxStars(
        camera,
        warpAmount
    );

    // ========================================================
    // MATRICES
    // ========================================================

    float aspectRatio =
        static_cast<float>(
            SCREEN_WIDTH
        )
        /
        static_cast<float>(
            SCREEN_HEIGHT
        );

    Mat4 view =
        getViewMatrix(
            camera
        );

    Mat4 projection =
        getProjectionMatrix(
            camera,
            aspectRatio
        );

    // ========================================================
    // SISTEMA SOLAR
    // ========================================================

    renderObject(
        sun,
        view,
        projection,
        elapsedTime
    );

    renderObject(
        rockyPlanet,
        view,
        projection,
        elapsedTime
    );

    renderObject(
        greenPlanet,
        view,
        projection,
        elapsedTime
    );

    renderObject(
        ringPlanet,
        view,
        projection,
        elapsedTime
    );

    renderObject(
        rings,
        view,
        projection,
        elapsedTime
    );

    renderObject(
        moon,
        view,
        projection,
        elapsedTime
    );

    // ========================================================
    // ESTRUCTURAS DEL PLANETA VERDE
    // ========================================================

    renderObject(
        surfaceBase,
        view,
        projection,
        elapsedTime
    );

    renderObject(
        surfaceBuilding,
        view,
        projection,
        elapsedTime
    );

    renderObject(
        surfaceTower,
        view,
        projection,
        elapsedTime
    );

    renderObject(
        surfaceBeacon,
        view,
        projection,
        elapsedTime
    );

    // ========================================================
    // NAVE
    // ========================================================

    renderObject(
        spaceship,
        view,
        projection,
        elapsedTime
    );

    // ========================================================
    // FRAMEBUFFER -> SDL
    // ========================================================

    RenderBuffer(
        renderer
    );

    SDL_RenderPresent(
        renderer
    );
}

// ============================================================
// MAIN
// ============================================================

int main(
    int argc,
    char* argv[]
) {
    if (
        !initSDL()
    ) {
        return 1;
    }

    if (
        !createScene()
    ) {
        SDL_DestroyRenderer(
            renderer
        );

        SDL_DestroyWindow(
            window
        );

        SDL_Quit();

        return 1;
    }

    // ========================================================
    // INFORMACION
    // ========================================================

    std::cout
        << std::endl;

    std::cout
        << "========================================"
        << std::endl;

    std::cout
        << "PROYECTO 3 - SPACE TRAVEL"
        << std::endl;

    std::cout
        << "========================================"
        << std::endl;

    std::cout
        << std::endl;

    std::cout
        << "CONTROLES:"
        << std::endl;

    std::cout
        << "W = Acelerar"
        << std::endl;

    std::cout
        << "S = Frenar / reversa"
        << std::endl;

    std::cout
        << "A / D = Girar"
        << std::endl;

    std::cout
        << "Q / E = Subir / bajar"
        << std::endl;

    std::cout
        << "SHIFT = Warp"
        << std::endl;

    std::cout
        << "ESC = Salir"
        << std::endl;

    std::cout
        << std::endl;

    std::cout
        << "Sistema:"
        << std::endl;

    std::cout
        << "- Sol animado"
        << std::endl;

    std::cout
        << "- Planeta rocoso"
        << std::endl;

    std::cout
        << "- Planeta verde con base"
        << std::endl;

    std::cout
        << "- Planeta con anillos"
        << std::endl;

    std::cout
        << "- Luna"
        << std::endl;

    std::cout
        << "- Skybox de estrellas"
        << std::endl;

    std::cout
        << "- Warp animado"
        << std::endl;

    std::cout
        << "- Colisiones"
        << std::endl;

    std::cout
        << std::endl;

    // ========================================================
    // TIEMPO
    // ========================================================

    Uint64 startTicks =
        SDL_GetTicks64();

    Uint64 previousTicks =
        startTicks;

    bool running =
        true;

    while (
        running
    ) {
        // ====================================================
        // EVENTOS
        // ====================================================

        SDL_Event event;

        while (
            SDL_PollEvent(
                &event
            )
        ) {
            if (
                event.type
                ==
                SDL_QUIT
            ) {
                running =
                    false;
            }

            if (
                event.type
                    ==
                    SDL_KEYDOWN
                &&
                event.key.keysym.sym
                    ==
                    SDLK_ESCAPE
            ) {
                running =
                    false;
            }
        }

        // ====================================================
        // TIEMPO REAL
        // ====================================================

        Uint64 currentTicks =
            SDL_GetTicks64();

        float elapsedTime =
            static_cast<float>(
                currentTicks
                -
                startTicks
            )
            /
            1000.0f;

        float deltaTime =
            static_cast<float>(
                currentTicks
                -
                previousTicks
            )
            /
            1000.0f;

        previousTicks =
            currentTicks;

        deltaTime =
            std::min(
                deltaTime,
                0.05f
            );

        // ====================================================
        // UPDATE
        // ====================================================

        updateScene(
            elapsedTime,
            deltaTime
        );

        // ====================================================
        // RENDER
        // ====================================================

        renderFrame(
            elapsedTime
        );

        SDL_Delay(
            16
        );
    }

    // ========================================================
    // SALIDA
    // ========================================================

    SDL_DestroyRenderer(
        renderer
    );

    SDL_DestroyWindow(
        window
    );

    SDL_Quit();

    return 0;
}