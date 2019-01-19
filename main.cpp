#define GLFW_INCLUDE_NONE
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <vector>
#include <random>
#include <tuple>
#include <array>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/color_space.hpp>
#include <stb_image.h>
#include <bass.h>

#undef near
#undef far

#include "RessourceManager.h"
#include "Mesh.h"
#include "Animation.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Effect.h"
#include "ParticleSystem.h"
#include "Scene.h"
#include "Actions.h"

using namespace std;
using namespace glm;

struct PointLight {
    glm::vec3 pos;
    glm::vec3 color;
    float brightness;
    float constantTerm;
    float linearTerm;
    float quadraticTerm;

    PointLight(glm::vec3 pos, glm::vec3 color, float brightness, float constantTerm, float linearTerm, float quadraticTerm) :
        pos(pos),
        color(color),
        brightness(brightness),
        constantTerm(constantTerm),
        linearTerm(linearTerm),
        quadraticTerm(quadraticTerm)
    {
    }
};

static bool debug_flag = false;
static bool fullscreen = false;

const int DEFAULT_WIDTH = 1920;
const int DEFAULT_HEIGHT = 1080;
const float CAMERA_SPEED = 10.0f;
const int NUM_MUSIC_CUBES = 36;

const double BEATS_PER_SECOND = 128.0 / 60.0;
const double BEAT_OFFSET = -1.032;

static int windowWidth = 0, windowHeight = 0;
static float frameRate;
static float animationTime;

static vector<PointLight> pointLights;
static Camera camera;
static float focus = 4;
static Program gBufferShader, particleShader, composeShader, ssdoShader;
static Program environmentShader;
static Program particleUpdateShader;

static Program gBufferRefractiveShader;
static Program gBufferLayer2Shader;
static Mesh lightMesh;
static bool muteSong = false;
static bool useAnimatedCamera = true;
static GLuint blurFBO0, blurFBO1;
static GLuint blurBuffer0, blurBuffer1;

GLFWwindow *initGLFW();
bool initGLEW();
GLuint getScreenQuadVAO();
GLuint generateTexture();
GLuint generateTexture(int width, int height);
void drawScreenQuad(GLuint screenQuadVAO);
void mouseCallback(GLFWwindow* window, double xPos, double yPos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

glm::vec3 getHemisphereSample(glm::vec2 u) {
    // source: "Physically Based Rendering: From Theory to Implementation" [Pharr and Humphreys, 2016]
    float r = std::sqrt(u.x);
    float phi = 2.0f * 3.1415926f * u.y;
    return glm::vec3(
        r * std::cos(phi),
        r * std::sin(phi),
        std::sqrt(std::max(0.0f, 1.0f - u.x))
    );
}

// generated using the method from "Sampling with Hammersley and Halton Points" [Wong et al., 1997]
// with parameters p1 = 2, p2 = 5, p3 = 7, interleaved
const float HALTON_POINTS[64 * 3] = {
    0.500f, 0.333f, 0.200f, 0.250f, 0.667f, 0.400f,
    0.750f, 0.111f, 0.600f, 0.125f, 0.444f, 0.800f,
    0.625f, 0.778f, 0.040f, 0.375f, 0.222f, 0.240f,
    0.875f, 0.556f, 0.440f, 0.062f, 0.889f, 0.640f,
    0.562f, 0.037f, 0.840f, 0.312f, 0.370f, 0.080f,
    0.812f, 0.704f, 0.280f, 0.188f, 0.148f, 0.480f,
    0.688f, 0.481f, 0.680f, 0.438f, 0.815f, 0.880f,
    0.938f, 0.259f, 0.120f, 0.031f, 0.593f, 0.320f,
    0.531f, 0.926f, 0.520f, 0.281f, 0.074f, 0.720f,
    0.781f, 0.407f, 0.920f, 0.156f, 0.741f, 0.160f,
    0.656f, 0.185f, 0.360f, 0.406f, 0.519f, 0.560f,
    0.906f, 0.852f, 0.760f, 0.094f, 0.296f, 0.960f,
    0.594f, 0.630f, 0.008f, 0.344f, 0.963f, 0.208f,
    0.844f, 0.012f, 0.408f, 0.219f, 0.346f, 0.608f,
    0.719f, 0.679f, 0.808f, 0.469f, 0.123f, 0.048f,
    0.969f, 0.457f, 0.248f, 0.016f, 0.790f, 0.448f,
    0.516f, 0.235f, 0.648f, 0.266f, 0.568f, 0.848f,
    0.766f, 0.901f, 0.088f, 0.141f, 0.049f, 0.288f,
    0.641f, 0.383f, 0.488f, 0.391f, 0.716f, 0.688f,
    0.891f, 0.160f, 0.888f, 0.078f, 0.494f, 0.128f,
    0.578f, 0.827f, 0.328f, 0.328f, 0.272f, 0.528f,
    0.828f, 0.605f, 0.728f, 0.203f, 0.938f, 0.928f,
    0.703f, 0.086f, 0.168f, 0.453f, 0.420f, 0.368f,
    0.953f, 0.753f, 0.568f, 0.047f, 0.198f, 0.768f,
    0.547f, 0.531f, 0.968f, 0.297f, 0.864f, 0.016f,
    0.797f, 0.309f, 0.216f, 0.172f, 0.642f, 0.416f,
    0.672f, 0.975f, 0.616f, 0.422f, 0.025f, 0.816f,
    0.922f, 0.358f, 0.056f, 0.109f, 0.691f, 0.256f,
    0.609f, 0.136f, 0.456f, 0.359f, 0.469f, 0.656f,
    0.859f, 0.802f, 0.856f, 0.234f, 0.247f, 0.096f,
    0.734f, 0.580f, 0.296f, 0.484f, 0.914f, 0.496f,
    0.984f, 0.062f, 0.696f, 0.008f, 0.395f, 0.896f
};

int main(int argc, const char** argv) {
    if (argc >= 2) {
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], "--debug") == 0) {
                debug_flag = true;
            }
            else if (strcmp(argv[i], "--mute") == 0) {
                muteSong = true;
                BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 0);
            }
            else if (strcmp(argv[i], "--fullscreen") == 0) {
                fullscreen = true;
            }
        }
    }

    GLFWwindow* window = initGLFW();
    if (window == nullptr) {
        return -1;
    }
    if (!initGLEW()) {
        return -1;
    }
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glEnable(GL_DEPTH_TEST);

    if (!BASS_Init(-1, 44100, 0, glfwGetWin32Window(window), NULL)) {
        std::cout << "Can't initialize bass";
        return 0;
    }

    DWORD bassStream;
    bassStream = BASS_StreamCreateFile(FALSE, "music/music1uncut.mp3", 0, 0, BASS_SAMPLE_LOOP);
    DWORD bassStreamDecoded = BASS_StreamCreateFile(FALSE, "music/music1uncut.mp3", 0, 0, BASS_SAMPLE_LOOP | BASS_STREAM_PRESCAN | BASS_STREAM_DECODE);
    if (!bassStream) {
        return 0;
    }

    std::default_random_engine random;

    GLuint gColor, gWorldPos, gNormal, gReflection, gEmission, gPrimitiveID, gDepth;
    GLuint gBuffer = generateFramebuffer(
        windowWidth, windowHeight, {
            {GL_COLOR_ATTACHMENT0, gColor, GL_RGB16F},
            {GL_COLOR_ATTACHMENT1, gWorldPos, GL_RGB16F},
            {GL_COLOR_ATTACHMENT2, gNormal, GL_RGB16F},
            {GL_COLOR_ATTACHMENT3, gReflection, GL_RGB16F},
            {GL_COLOR_ATTACHMENT4, gEmission, GL_RGB16F},
            {GL_COLOR_ATTACHMENT5, gPrimitiveID, GL_R8},
            {GL_DEPTH_ATTACHMENT, gDepth, GL_DEPTH_COMPONENT24},
        }, {
        }
        );

    GLuint lowResDepth;
    GLuint lowResFramebuffer = generateFramebuffer(
        windowWidth / 4, windowHeight / 4, {
            {GL_DEPTH_ATTACHMENT, lowResDepth, GL_DEPTH_COMPONENT24}
        }, {}
    );

    GLuint filteredBuffer;
    GLuint filterFramebuffer = generateFramebuffer(
        windowWidth, windowHeight, {
            {GL_COLOR_ATTACHMENT0, filteredBuffer, GL_RGB16F}
        }, {}
    );

    GLuint gColorRefractive, gWorldPosRefractive, gNormalRefractive,
        gReflectionRefractive, gEmissionRefractive, gOppositePos,
        gDepthRefractive, gRefraction;
    GLuint gBufferRefractive = generateFramebuffer(
        windowWidth, windowHeight, {
            {GL_COLOR_ATTACHMENT0, gColorRefractive, GL_RGB16F},
            {GL_COLOR_ATTACHMENT1, gWorldPosRefractive, GL_RGB16F},
            {GL_COLOR_ATTACHMENT2, gNormalRefractive, GL_RGB16F},
            {GL_COLOR_ATTACHMENT3, gReflectionRefractive, GL_RGB16F},
            {GL_COLOR_ATTACHMENT4, gEmissionRefractive, GL_RGB16F},
            {GL_COLOR_ATTACHMENT5, gOppositePos, GL_RGB16F},
            {GL_COLOR_ATTACHMENT6, gRefraction, GL_RGB16F},
            {GL_DEPTH_ATTACHMENT, gDepthRefractive, GL_DEPTH_COMPONENT24},
        }, {
        }
        );

    GLuint gWorldPosLayer2, gNormalLayer2, gDepthLayer2;
    GLuint gBufferLayer2 = generateFramebuffer(
        windowWidth, windowHeight, {
            {GL_COLOR_ATTACHMENT0, gWorldPosLayer2, GL_RGB16F},
            {GL_COLOR_ATTACHMENT1, gNormalLayer2, GL_RGB16F},
            {GL_DEPTH_ATTACHMENT, gDepthLayer2, GL_DEPTH_COMPONENT24},
        }, {
        }
        );

    GLuint environmentColor, environmentDepth;
    GLuint cubemapFramebuffer = generateFramebuffer(
        256, 256, GL_TEXTURE_CUBE_MAP, {
            {GL_COLOR_ATTACHMENT0, environmentColor, GL_RGB16F},
            {GL_DEPTH_ATTACHMENT, environmentDepth, GL_DEPTH_COMPONENT24},
        }, {
        }
        );

    GLuint multisampleColor, multisampleDepth;
    GLuint multisampleBuffer = generateFramebufferMultisample(
        windowWidth, windowHeight, 4, {
            {GL_COLOR_ATTACHMENT0, multisampleColor, GL_RGB16F},
        }, {
            {GL_DEPTH_ATTACHMENT, multisampleDepth, GL_DEPTH_COMPONENT24},
        }
        );

    GLuint blurredEnvironmentColor;
    GLuint blurredEnvironment = generateFramebuffer(
        16, 16, GL_TEXTURE_CUBE_MAP, {
            {GL_COLOR_ATTACHMENT0, blurredEnvironmentColor, GL_RGB16F},
        }, {}
    );

    Program blurCube(
        "shaders/cubeBlur.vert", "shaders/cubeBlur.geom",
        "shaders/cubeBlur.frag"
    );
    blurCube.use();
    glUniform1i(glGetUniformLocation(blurCube.program, "cubemap"), 0);

    tuple<vec2, vec3, float> cubeMesh[24] = {
        // X+
        tuple<vec2, vec3, float>{ {-1, -1}, {+1, +1, +1}, 0},
        tuple<vec2, vec3, float>{ {+1, -1}, {+1, +1, -1}, 0},
        tuple<vec2, vec3, float>{ {-1, +1}, {+1, -1, +1}, 0},
        tuple<vec2, vec3, float>{ {+1, +1}, {+1, -1, -1}, 0},
        // X-
        tuple<vec2, vec3, float>{ {-1, -1}, {-1, +1, -1}, 1},
        tuple<vec2, vec3, float>{ {+1, -1}, {-1, +1, +1}, 1},
        tuple<vec2, vec3, float>{ {-1, +1}, {-1, -1, -1}, 1},
        tuple<vec2, vec3, float>{ {+1, +1}, {-1, -1, +1}, 1},
        // Y+
        tuple<vec2, vec3, float>{ {-1, -1}, {-1, +1, -1}, 2},
        tuple<vec2, vec3, float>{ {+1, -1}, {+1, +1, -1}, 2},
        tuple<vec2, vec3, float>{ {-1, +1}, {-1, +1, +1}, 2},
        tuple<vec2, vec3, float>{ {+1, +1}, {+1, +1, +1}, 2},
        // Y-
        tuple<vec2, vec3, float>{ {-1, -1}, {-1, -1, +1}, 3},
        tuple<vec2, vec3, float>{ {+1, -1}, {+1, -1, +1}, 3},
        tuple<vec2, vec3, float>{ {-1, +1}, {-1, -1, -1}, 3},
        tuple<vec2, vec3, float>{ {+1, +1}, {+1, -1, -1}, 3},
        // Z+
        tuple<vec2, vec3, float>{ {-1, -1}, {-1, +1, +1}, 4},
        tuple<vec2, vec3, float>{ {+1, -1}, {+1, +1, +1}, 4},
        tuple<vec2, vec3, float>{ {-1, +1}, {-1, -1, +1}, 4},
        tuple<vec2, vec3, float>{ {+1, +1}, {+1, -1, +1}, 4},
        // Z-
        tuple<vec2, vec3, float>{ {-1, -1}, {+1, +1, -1}, 5},
        tuple<vec2, vec3, float>{ {+1, -1}, {-1, +1, -1}, 5},
        tuple<vec2, vec3, float>{ {-1, +1}, {+1, -1, -1}, 5},
        tuple<vec2, vec3, float>{ {+1, +1}, {-1, -1, -1}, 5},
    };
    unsigned char cubeIndex[36] = {
        0, 1, 2, 2, 1, 3,  4, 5, 6, 6, 5, 7,
        8, 9, 10, 10, 9, 11,  12, 13, 14, 14, 13, 15,
        16, 17, 18, 18, 17, 19,  20, 21, 22, 22, 21, 23
    };
    GLuint cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    GLuint cubeVBO, cubeIndexVBO;
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeMesh), &cubeMesh, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * 4, (GLvoid*)16);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * 4, (GLvoid*)(4));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * 4, (GLvoid*)(0));

    glGenBuffers(1, &cubeIndexVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexVBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndex), &cubeIndex, GL_STATIC_DRAW
    );


    GLuint screenQuadVAO = getScreenQuadVAO();

    Scene environmentScene, mainScene;

    MeshInfo musicCubeMeshInfo("scenes/scene1/", "MusicCube.obj");
    MeshInfo floorMeshInfo("scenes/scene1/", "Floor.obj");
    MeshInfo mirrorsMeshInfo("scenes/scene1/", "Mirrors.obj");
    MeshInfo centerCubeMeshInfo("scenes/scene1/", "CenterCube.obj");
    MeshInfo lightRimInfo("scenes/scene1/", "LightRim.obj");
    MeshInfo dragonGlassMeshInfo("scenes/scene3/", "Suzanne.vbo");
    MeshInfo lucyGlassMeshInfo("scenes/scene3/", "Lucy.vbo");

    Mesh lightRimObject = Mesh(
        lightRimInfo,
        glm::vec3(0.0f),
        0.0f,
        glm::vec3(2.0f)
    );

    mainScene.objects.insert(&lightRimObject);
    environmentScene.objects.insert(&lightRimObject);

    Mesh floorObject = Mesh(
        floorMeshInfo,
        glm::vec3(1.0f),
        0.0f,
        glm::vec3(0.0f),
        glm::vec3(0.0f),
        glm::vec3(80.0f / 144.0f)
    );
    Mesh mirrorsObject = Mesh(
        mirrorsMeshInfo,
        glm::vec3(0.1f),
        0.0f,
        glm::vec3(0.0f),
        glm::vec3(0.0f),
        glm::vec3(80.0f / 144.0f)
    );

    mainScene.objects.insert(&floorObject);
    mainScene.objects.insert(&mirrorsObject);

    ParticleSystem particles(100000, 3, 4, 5);

    std::normal_distribution<float> normalFloats;
    //for (int i = 0; i < 10000; i++) {
    //    particles.add(
    //        {
    //            normalFloats(random) * 5,
    //            0.1,
    //            normalFloats(random) * 5
    //        }, glm::normalize(glm::quat(
    //            normalFloats(random),
    //            normalFloats(random),
    //            normalFloats(random),
    //            normalFloats(random)
    //        ))
    //    );
    //}

    Mesh musicCubes[NUM_MUSIC_CUBES];

    Mesh dragonGlassMesh = Mesh(
        dragonGlassMeshInfo,
        glm::vec3(1.0f, 1.0f, 1.0f),
        0.0f,
        glm::vec3(0.0f),
        glm::vec3(10.0f, 30.0f, 0.0f),
        glm::vec3(1.0f)
    );
    Mesh lucyGlassMesh = Mesh(
        lucyGlassMeshInfo,
        glm::vec3(1.0f, 1.0f, 1.0f),
        0.0f,
        glm::vec3(0.0f),
        glm::vec3(10.0f, 30.0f, 0.0f),
        glm::vec3(1.0f)
    );
    
    mainScene.glassObjects.insert(&dragonGlassMesh);
    mainScene.glassObjects.insert(&lucyGlassMesh);

    const float lightFloorOffset = 2.0f;
    for (int i = 0; i < 36; i++) {
        auto pos = glm::vec3(sinf(glm::radians(i * 10.0f)) * 20.0f, 0.0f, cosf(glm::radians(i * 10.0f)) * 20.0f);
        auto scale = glm::vec3(1.0f, sin(i / 2.0f) * 0.3f + 1.5f, 1.0f);
        auto rotation = glm::radians(i * 10.0f);

        musicCubes[i] = Mesh(
            musicCubeMeshInfo,
            glm::vec3(0.0f),
            1.0f,
            glm::vec3(1.0f),
            pos,
            scale,
            glm::vec3(0.0f, rotation, 0.0f)
        );
        musicCubes[i].minHeight = sin(i / 2.0f) * 0.1f + 0.5f;
        musicCubes[i].maxHeight = scale.y;

        mainScene.objects.insert(&musicCubes[i]);
        environmentScene.objects.insert(&musicCubes[i]);

        auto model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);
        pointLights.push_back(PointLight(
            model * glm::vec4(0.0f, lightFloorOffset, 0.0f, 1.0f),
            glm::vec3(1.0f),
            2.0f,
            1.0f,
            0.07f,
            4.0f
        ));
    }

    Mesh centerCubes[8];
    for (int i = 0; i < 8; i++) {
        centerCubes[i] = Mesh(
            centerCubeMeshInfo,
            glm::vec3(1.0f, 1.0f, 1.0f) * 2.0f,
            0.0f,
            glm::vec3(0.0f)
        );
        mainScene.objects.insert(&centerCubes[i]);
    }

    float near = 0.5f;
    float far = 200.0f;
    float fov = glm::radians(45.0f);
    glm::mat4 projectionMatrix = glm::perspective(
        fov, (float)windowWidth / (float)windowHeight, near, far
    );

    Animation<glm::vec3> cameraPosition{
        {0, {34, 3, -35}, HandleType::SMOOTH_OUT},
        {8, {34, 1.5, -25}, HandleType::SMOOTH},
        {16, {32, 1, -10}, HandleType::SMOOTH_IN},

        {16, {34, 0.7, 1.5}, HandleType::SMOOTH_OUT},
        {32, {10, 0.7, 1.5}, HandleType::SMOOTH_IN},

        {32, {25, 2, -36}, HandleType::SMOOTH_OUT},
        {48, {-25, 2, -36}, HandleType::SMOOTH_IN},

        {48, {-17, 1.5, -1}, HandleType::SMOOTH_OUT},
        {64, {17, 1.5, -1}, HandleType::SMOOTH_IN},

        {64 + 0, {2.5, 5, -2.2}, HandleType::SMOOTH_OUT},
        {64 + 8, {3.6, 1.1, 3.6}, HandleType::SMOOTH_IN},

        {64 + 8, {3.6, 1.1, -2.2}, HandleType::SMOOTH_OUT},
        {64 + 16, {2.5, 5, 3.6}, HandleType::SMOOTH_IN},

        {64 + 16, {34, 3.7, 0}, HandleType::SMOOTH_OUT},
        {64 + 32, {10, 3.7, 0}, HandleType::SMOOTH_IN},

        {96, {28.9201, 3.66179, 2.78608}, HandleType::SMOOTH_OUT},
        {96 + 8, {3.02361, 28.411, 31.7517}, HandleType::SMOOTH_IN},

        {96 + 8, {10.7965, 2.20677, 39.3255}, HandleType::SMOOTH_OUT},
        {96 + 16, {7.54126, 33.2029, 31.4303}, HandleType::SMOOTH_IN},

        {96 + 16, {-13.7066, 2.50239, -26.2294}, HandleType::SMOOTH_OUT},
        {96 + 24, {-28.9809, 2.50239, 1.32618}, HandleType::SMOOTH_IN},

        {96 + 24, {44, 3.7, 0}, HandleType::SMOOTH_OUT},
        {96 + 32, {-44, 3.7, -10}, HandleType::SMOOTH_IN},

        {128 + 0, {-13.1675, 1.35622, -12.8348}, HandleType::SMOOTH_OUT},
        {128 + 8, {-7.43895, 52.3118, 9.91848}, HandleType::SMOOTH_IN},

        {128 + 8, {10.5, 0.3, 14.0}, HandleType::SMOOTH_OUT},
        {128 + 16, {10.5, 0.3, -14.0}, HandleType::SMOOTH_IN},

        {128 + 16, {18, 8.2, 3.3}, HandleType::SMOOTH_OUT},
        {128 + 24, {18, 8.2, 3.3}, HandleType::SMOOTH_IN},

        {128 + 24, {6.5, 0.5, -24.6}, HandleType::SMOOTH_OUT},
        {128 + 32, {-5.3, 0.5, 19}, HandleType::SMOOTH_IN},

        {160 + 0, {0.5, 1.5, -4.5}, HandleType::SMOOTH_OUT},
        {160 + 8, {-4.5, 1.5, 0}},
        {160 + 16, {-0.5, 1.5, 4.5}, HandleType::SMOOTH_IN},

        {96 + 32, {-13.1675, 1.35622, -12.8348}, HandleType::SMOOTH_OUT},
        {96 + 40, {-7.43895, 52.3118, 9.91848}, HandleType::SMOOTH_IN},

        {160 + 16, {17, 1.5, 4}, HandleType::SMOOTH_OUT},
        {160 + 32, {4, 1.5, 15}, HandleType::SMOOTH_IN},

        {192, {-28.3626, 14.7267, -24.2202}, HandleType::SMOOTH_OUT},
        {205, {-28.3626, 8.7267, -24.2202}, HandleType::SMOOTH_IN},

        // dragon descend
        {200, {13.1993, 6.39421, -9.93616}, HandleType::STOP},
        {216, {13.1993, 6.39421, -9.93616}, HandleType::STOP},
        {232, {0, 3, -5}, HandleType::STOP},

        // lucy descend
        {232, {10, 9, 0}, HandleType::STOP},
        {248, {10, 9, 0}, HandleType::STOP},
        {264, {-10, 10, -10}, HandleType::STOP},

        {264, {25, 2, -36}, HandleType::SMOOTH_OUT},
        {280, {-25, 2, -36}, HandleType::SMOOTH_IN},

        {280, {-14.9888, 3.52172, -1.13982}, HandleType::SMOOTH_OUT},
        {288, {-10.5713, 6.68541, 4.54254}, HandleType::SMOOTH_IN},
        {296, {-4.56759, 9.84052, -0.222573}, HandleType::SMOOTH_IN},

        {296, {2.01516, 2.5405, 4.53035}, HandleType::SMOOTH_OUT},
        {312, {2.01516, 5.5405, 4.53035}, HandleType::SMOOTH_IN},

        {312, {34, 0.7, 1.5}, HandleType::SMOOTH_OUT},
        {328, {10, 0.7, 1.5}, HandleType::SMOOTH_IN},

        {328, {2.01516, 5.5405, 4.53035}, HandleType::SMOOTH_IN},
        {328, {-14.3007, 27.9264, -7.8107}, HandleType::SMOOTH_OUT},
        {344, {-14.3007, 1.7569, -7.8107}, HandleType::SMOOTH_IN},
    };

    Animation<glm::vec3> cameraFocus{
        {0, {34, 0, -33}, HandleType::SMOOTH_OUT},
        {8, {32, 0, -23}},
        {16, {20, 0, 0}, HandleType::SMOOTH_IN},

        {16, {20, 0.7, 1.5}, HandleType::SMOOTH_OUT},
        {32, {-4, 0.7, 1.5}, HandleType::SMOOTH_IN},

        {32, {25, 2, -10}, HandleType::SMOOTH_OUT},
        {48, {-25, 2, -10}, HandleType::SMOOTH_IN},

        {48, {-17, 0, -7}, HandleType::SMOOTH_OUT},
        {64, {17, 0, -7}, HandleType::SMOOTH_IN},

        {64 + 0, {0, 2, 0}, HandleType::SMOOTH_OUT},
        {64 + 16, {0, 2, 0}, HandleType::SMOOTH_IN},

        {64 + 16, {20, 3.7, 0}, HandleType::SMOOTH_OUT},
        {64 + 32, {-4, 3.7, 0}, HandleType::SMOOTH_IN},

        {96, {0, 0, 0}, HandleType::STOP},
        {96 + 16, {0, 0, 0}, HandleType::STOP},

        {96 + 16, {-5.00671, 1.47447, -21.407}, HandleType::SMOOTH_OUT},
        {96 + 24, {-20.281, 1.47447, 6.14859}, HandleType::SMOOTH_IN},

        {96 + 24, {0, 2, 0}, HandleType::SMOOTH_OUT},
        {96 + 32, {0, 2, 0}, HandleType::SMOOTH_IN},

        {128 + 0, {0, 2, 0}, HandleType::SMOOTH_OUT},
        {128 + 8, {0, 2, 0}, HandleType::SMOOTH_IN},

        {128 + 8, {20, 3.3, 0}, HandleType::SMOOTH_OUT},
        {128 + 16, {20, 3.3, 0}, HandleType::SMOOTH_IN},

        {128 + 16, {19.5, 5.2, 3.3}, HandleType::SMOOTH_OUT},
        {128 + 24, {19.5, 5.2, 3.3}, HandleType::SMOOTH_IN},

        {128 + 24, {4.5, 0.5, -20.6}, HandleType::SMOOTH_OUT},
        {128 + 32, {-4.3, 0.5, 20}, HandleType::SMOOTH_IN},

        {160 + 0, {0, 2, 0}, HandleType::SMOOTH_OUT},
        {160 + 16, {0, 2, 0}, HandleType::SMOOTH_IN},

        {160 + 16, {11, 0, 6}, HandleType::SMOOTH_OUT},
        {160 + 32, {2, 0, 17}, HandleType::SMOOTH_IN},

        {192, {0, 0, 0}, HandleType::SMOOTH_OUT},
        {205, {0, 0, 0}, HandleType::SMOOTH_IN},

        // dragon descend
        {205, {10, 30, 0}, HandleType::STOP},
        {216, {10, 1, 0}, HandleType::STOP},
        {232, {7.47423, 3.34526, 0.157206}, HandleType::STOP},

        // lucy descend
        {232, {-10, 30, 0}, HandleType::STOP},
        {248, {-9.4947, 4.94998, -0.39352}, HandleType::STOP},
        {264, {-9.4947, 4.94998, -0.39352}, HandleType::STOP},

        {264, {25, 2, -10}, HandleType::SMOOTH_OUT},
        {280, {-25, 2, -10}, HandleType::SMOOTH_IN},

        {280, {-10.1789, 3.06619, -1.18164}, HandleType::SMOOTH_OUT},
        {288, {-9.87901, 5.69849, 1.07871}, HandleType::SMOOTH_IN},
        {296, {-8.89624, 7.34594, -0.204375}, HandleType::SMOOTH_IN},

        {296, { 13.5958, 3.06248, -0.385165}, HandleType::SMOOTH_OUT},
        {312, { 6.28133, 3.99602, 2.71954}, HandleType::SMOOTH_IN},

        {312, {20, 0.7, 1.5}, HandleType::SMOOTH_OUT},
        {328, {-4, 0.7, 1.5}, HandleType::SMOOTH_IN},

        {328, { 6.28133, 3.99602, 2.71954}, HandleType::SMOOTH_IN},
        {328, {0, 0, 0}, HandleType::SMOOTH_OUT},
    };

    Placement lightRimSwipeStart = {{ 0, -3, 0 }};
    Placement lightRimSwipeEnd = {{ 0, 15, 0 }};
    Animation<Placement> lightRimAnimation{
        {
            {112, lightRimSwipeStart, HandleType::STOP},
            {113, lightRimSwipeEnd, HandleType::STOP},
            {113, lightRimSwipeStart, HandleType::STOP},
            {114, lightRimSwipeEnd, HandleType::STOP},
            {114, lightRimSwipeStart, HandleType::STOP},
            {115, lightRimSwipeEnd, HandleType::STOP},
            {115, lightRimSwipeStart, HandleType::STOP},
            {116, lightRimSwipeEnd, HandleType::STOP},
            {116, lightRimSwipeStart, HandleType::STOP},
            {117, lightRimSwipeEnd, HandleType::STOP},
            {117, lightRimSwipeStart, HandleType::STOP},
            {118, lightRimSwipeEnd, HandleType::STOP},
            {118, lightRimSwipeStart, HandleType::STOP},
            {119, lightRimSwipeEnd, HandleType::STOP},
            {119, lightRimSwipeStart, HandleType::STOP},
            {120, lightRimSwipeEnd, HandleType::STOP},
            {120, lightRimSwipeStart, HandleType::STOP},
            {121, lightRimSwipeEnd, HandleType::STOP},
            {121, lightRimSwipeStart, HandleType::STOP},
            {122, lightRimSwipeEnd, HandleType::STOP},
            {122, lightRimSwipeStart, HandleType::STOP},
            {123, lightRimSwipeEnd, HandleType::STOP},
            {123, lightRimSwipeStart, HandleType::STOP},
            {124, lightRimSwipeEnd, HandleType::STOP},
            {124, lightRimSwipeStart, HandleType::STOP},

            {192, lightRimSwipeStart, HandleType::STOP},
            {200, {{ 0, 25, 0 }}, HandleType::STOP},
        }
    };

    Animation<vec3> centerCubeOffset{
        {
            {24, CLAP},

            {64 + 0, CLAP},
            {64 + 2, CLAP},
            {64 + 6, CLAP},
            {64 + 8, CLAP},
            {64 + 10, CLAP},
            {64 + 14, CLAP},
        }
    };

    Animation<vec3> centerCubeRotation{
        {
            {29, RUBIKS_Y},
            {30, RUBIKS_X},
            {31, RUBIKS_Z},

            {64 + 4, RUBIKS_Z},
            {64 + 5, RUBIKS_X},
            {64 + 12, RUBIKS_Z},
            {64 + 13, RUBIKS_X},
        }
    };

    // https://jsfiddle.net/d6mj4tfo/
    Animation<glm::vec3> lightRimColorAnimation{
        {
            {0, {1,1,1}, HandleType::STOP},
            {112, {1,1,1}, HandleType::STOP},
            {112, {1,0,0}, HandleType::STOP},
            {118, {0,1,0}, HandleType::STOP},
            {124, {0,0,1}, HandleType::STOP},
            {124, {1,1,1}, HandleType::STOP},

            {200, {1,1,1}, HandleType::STOP},
            {216, {0.45, 0.75, 1}, HandleType::STOP},
        }
    };

    Animation<float> lightRimBrightnessAnimation{
        {
            {0, 1, HandleType::STOP},
            {80, 1, HandleType::STOP},
            {96, 0, HandleType::STOP},
            {112, 0, HandleType::STOP},

            {112, 3, HandleType::STOP},
            {113, 0, HandleType::STOP},
            {113, 3, HandleType::STOP},
            {114, 0, HandleType::STOP},
            {114, 3, HandleType::STOP},
            {115, 0, HandleType::STOP},
            {115, 3, HandleType::STOP},
            {116, 0, HandleType::STOP},
            {116, 3, HandleType::STOP},
            {117, 0, HandleType::STOP},
            {117, 3, HandleType::STOP},
            {118, 0, HandleType::STOP},
            {118, 3, HandleType::STOP},
            {119, 0, HandleType::STOP},
            {119, 3, HandleType::STOP},
            {120, 0, HandleType::STOP},
            {120, 3, HandleType::STOP},
            {121, 0, HandleType::STOP},
            {121, 3, HandleType::STOP},
            {122, 0, HandleType::STOP},
            {122, 3, HandleType::STOP},
            {123, 0, HandleType::STOP},
            {123, 3, HandleType::STOP},
            {124, 0, HandleType::STOP},
            {124, 3, HandleType::STOP},
            {124, 0, HandleType::STOP},

            {200, 0, HandleType::STOP},
            {216, 2.5, HandleType::STOP},

            {328, 2.5, HandleType::STOP},
            {344, 0, HandleType::STOP},
        }
    };

    Animation<float> introMusicCubeAnimation{
        {
            {0, 1, HandleType::STOP},
            {80, 1, HandleType::STOP},
            {96, 0, HandleType::STOP},
            {192, 0, HandleType::STOP},
            {200, 1, HandleType::STOP},
            {200, 0, HandleType::STOP},
            {328, 0, HandleType::STOP},
            {328, 1, HandleType::STOP},
            {344, 0, HandleType::STOP},
        }
    };

    Animation<float> musicCubeAnimation{
        {
            {0, 0, HandleType::STOP},
            {96, 0, HandleType::STOP},
            {96, 1, HandleType::STOP},
            {192, 1, HandleType::STOP},
            {192, 0, HandleType::STOP},
            {344, 0, HandleType::STOP},
            {344, 1, HandleType::STOP},
        }
    };

    Animation<glm::vec3> dragonAnimation{
        {
            {200, {10, 50, 0}, HandleType::STOP},
            {216, {10, 0, 0}, HandleType::STOP},
        }
    };
    Animation<glm::vec3> lucyAnimation{
        {
            {232, {-10, 50, 0}, HandleType::STOP},
            {248, {-10, 0, 0}, HandleType::STOP},
        }
    };

    gBufferShader = Program("shaders/gBuffer.vert", "shaders/gBuffer.frag");
    environmentShader = Program(
        "shaders/environment.vert", "shaders/environment.geom",
        "shaders/environment.frag"
    );
    gBufferRefractiveShader = Program("shaders/gBufferRefractive.vert", "shaders/gBufferRefractive.frag");
    gBufferLayer2Shader = Program("shaders/gBufferDepthLayer2.vert", "shaders/gBufferDepthLayer2.frag");
    composeShader = Program("shaders/compose.vert", "shaders/compose.frag");

    particleShader = Program("shaders/particle.vert", "shaders/particle.frag");

    particleUpdateShader = Program("shaders/particleUpdate.comp");
    particleUpdateShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particles.instanceVbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particles.physicVbo);

    GLuint ssdoUnblurredTexture, ssdoTexture, noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB8, 8, 8, 0, GL_RGB, GL_FLOAT, HALTON_POINTS
    );
    auto ssdoPass = Effect(
        "shaders/ssdo.frag", windowWidth, windowHeight,
        {
            {"noiseTex", GL_TEXTURE_2D, noiseTexture},
            {"environmentColor", GL_TEXTURE_CUBE_MAP, blurredEnvironmentColor},
            {"gColorTex", GL_TEXTURE_2D, gColor},
            {"gNormalTex", GL_TEXTURE_2D, gNormal},
            {"gDepthTex", GL_TEXTURE_2D, gDepth},
            {"lowResDepthTex", GL_TEXTURE_2D, lowResDepth},
            {"gEmissionTex", GL_TEXTURE_2D, gEmission},
            {"gWorldPosTex", GL_TEXTURE_2D, gWorldPos},
        },
        { {"color", ssdoUnblurredTexture, GL_RGB16F} }
    );

    GLuint ssdoBlurHorizontalTexture;
    auto blurSSDOHorizontal = Effect(
        "shaders/blurSSDOHorizontal.frag", windowWidth, windowHeight,
        {
            {"colorTex", GL_TEXTURE_2D, ssdoUnblurredTexture},
            {"gNormalTex", GL_TEXTURE_2D, gNormal},
            {"gDepthTex", GL_TEXTURE_2D, gDepth},
        },
        { {"color", ssdoBlurHorizontalTexture, GL_RGB16F} }
    );
    auto blurSSDOVertical = Effect(
        "shaders/blurSSDOVertical.frag", windowWidth, windowHeight,
        {
            {"colorTex", GL_TEXTURE_2D, ssdoBlurHorizontalTexture},
            {"gNormalTex", GL_TEXTURE_2D, gNormal},
            {"gDepthTex", GL_TEXTURE_2D, gDepth},
        },
        { {"color", ssdoTexture, GL_RGB16F} }
    );

    GLuint backfaceRefraction, backfacePos;
    auto refractivePass = Effect(
        "shaders/glassMaterial.frag", windowWidth, windowHeight,
        { 
            {"ssdoTex", GL_TEXTURE_2D, ssdoTexture},
            {"gWorldPosTex", GL_TEXTURE_2D, gWorldPos},
            {"gNormalTex", GL_TEXTURE_2D, gNormal},

            {"gWorldPosRefractiveTex", GL_TEXTURE_2D, gWorldPosRefractive},
            {"gNormalRefractiveTex", GL_TEXTURE_2D, gNormalRefractive},
            {"gOppositePosTex", GL_TEXTURE_2D, gOppositePos},
            {"gRefractionTex", GL_TEXTURE_2D, gRefraction},

            {"gWorldPosLayer2Tex", GL_TEXTURE_2D, gWorldPosLayer2},
            {"gNormalLayer2Tex", GL_TEXTURE_2D, gNormalLayer2},
        },
        {
            {"backfaceRefractionOut", backfaceRefraction, GL_RGB16F},
            {"backfacePosOut", backfacePos, GL_RGB16F},
        }
    );

    GLuint ssrTexture;
    auto ssrPass = Effect(
        "shaders/ssr.frag", windowWidth, windowHeight,
        {
          {"gColorTex", GL_TEXTURE_2D, ssdoTexture},
          {"gNormalTex", GL_TEXTURE_2D, gNormal},
          {"gWorldPosTex", GL_TEXTURE_2D, gWorldPos},
          {"gReflectionTex", GL_TEXTURE_2D, gReflection},
          {"environmentColor", GL_TEXTURE_CUBE_MAP, blurredEnvironmentColor},
          {"backfaceRefractionTex", GL_TEXTURE_2D, backfaceRefraction},
          {"backfacePosTex", GL_TEXTURE_2D, backfacePos},
          {"gRefractionTex", GL_TEXTURE_2D, gRefraction},
          {"gWorldPosRefractive", GL_TEXTURE_2D, gWorldPosRefractive},
          {"gNormalRefractive", GL_TEXTURE_2D, gNormalRefractive}
        },
        { {"color", ssrTexture, GL_RGB16F} }
    );

    Program antiAliasingProgram(
        "shaders/antiAlias.vert", "shaders/antiAlias.frag"
    );
    antiAliasingProgram.use();
    glUniform1i(
        glGetUniformLocation(antiAliasingProgram.program, "colorTexture"), 3
    );
    glUniform1i(
        glGetUniformLocation(antiAliasingProgram.program, "gPrimitiveIDTex"), 4
    );

    GLuint bloomHorizontalTexture, bloomTexture;
    auto bloomHorizontalPass = Effect(
        "shaders/bloomHorizontal.frag", windowWidth / 2, windowHeight / 2,
        { {"colorTex", GL_TEXTURE_2D, ssrTexture} },
        { {"color", bloomHorizontalTexture, GL_RGB16F} }
    );
    auto bloomVerticalPass = Effect(
        "shaders/bloomVertical.frag", windowWidth / 2, windowHeight / 2,
        { {"colorTex", GL_TEXTURE_2D, bloomHorizontalTexture} },
        { {"color", bloomTexture, GL_RGB16F} }
    );

    GLuint dofCocTexture, dofCoarseTexture, dofTexture;
    auto dofCocPass = Effect(
        "shaders/dofCoc.frag", windowWidth, windowHeight, {
            {"depthTex", GL_TEXTURE_2D, gDepthRefractive},
        }, {
            {"coc", dofCocTexture, GL_R8_SNORM},
        }
    );
    auto dofCoarsePass = Effect(
        "shaders/dofCoarse.frag", windowWidth, windowHeight, {
            {"colorTex", GL_TEXTURE_2D, ssrTexture},
            {"cocTex", GL_TEXTURE_2D, dofCocTexture},
        }, {
            {"coarse", dofCoarseTexture, GL_RGBA16F},
        }
    );
    auto dofFinePass = Effect(
        "shaders/dofFine.frag", windowWidth, windowHeight, {
            {"coarseTex", GL_TEXTURE_2D, dofCoarseTexture},
        }, {
            {"color", dofTexture, GL_RGB16F},
        }
    );

    auto composePass = Effect(
        "shaders/compose.frag", windowWidth, windowHeight, {
            {"dofTex", GL_TEXTURE_2D, filteredBuffer},
            {"bloomTex", GL_TEXTURE_2D, bloomTexture},
        },
        0
    );

    gBufferShader.use();
    for (int i = 0; i < pointLights.size(); i++) {
        gBufferShader.setFloat("pointLights[" + std::to_string(i) + "].constantTerm", pointLights[i].constantTerm);
        gBufferShader.setFloat("pointLights[" + std::to_string(i) + "].linearTerm", pointLights[i].linearTerm);
        gBufferShader.setFloat("pointLights[" + std::to_string(i) + "].quadraticTerm", pointLights[i].quadraticTerm);
    }

    particleShader.use();
    for (int i = 0; i < pointLights.size(); i++) {
        particleShader.setFloat("pointLights[" + std::to_string(i) + "].constantTerm", pointLights[i].constantTerm);
        particleShader.setFloat("pointLights[" + std::to_string(i) + "].linearTerm", pointLights[i].linearTerm);
        particleShader.setFloat("pointLights[" + std::to_string(i) + "].quadraticTerm", pointLights[i].quadraticTerm);
    }

    camera = Camera(glm::vec3(0.0f, 1.0f, 0.0f));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    ssrPass.shader.use();
    glUniform2f(glGetUniformLocation(ssrPass.shader.program, "size"), windowWidth, windowHeight);

    refractivePass.shader.use();
    glUniform2f(glGetUniformLocation(refractivePass.shader.program, "size"), windowWidth, windowHeight);

    gBufferShader.use();
    for (int i = 0; i < pointLights.size(); i++) {
        gBufferShader.setFloat("pointLights[" + std::to_string(i) + "].brightness", pointLights[i].brightness);
        gBufferShader.setFloat("pointLights[" + std::to_string(i) + "].constantTerm", pointLights[i].constantTerm);
        gBufferShader.setFloat("pointLights[" + std::to_string(i) + "].linearTerm", pointLights[i].linearTerm);
        gBufferShader.setFloat("pointLights[" + std::to_string(i) + "].quadraticTerm", pointLights[i].quadraticTerm);
    }
    gBufferRefractiveShader.use();
    for (int i = 0; i < pointLights.size(); i++) {
        gBufferRefractiveShader.setFloat("pointLights[" + std::to_string(i) + "].brightness", pointLights[i].brightness);
        gBufferRefractiveShader.setFloat("pointLights[" + std::to_string(i) + "].constantTerm", pointLights[i].constantTerm);
        gBufferRefractiveShader.setFloat("pointLights[" + std::to_string(i) + "].linearTerm", pointLights[i].linearTerm);
        gBufferRefractiveShader.setFloat("pointLights[" + std::to_string(i) + "].quadraticTerm", pointLights[i].quadraticTerm);
    }

    particleUpdateShader.use();
    particleUpdateShader.setVector3f("attractorPosition", glm::vec3(0.0f, 0.1f, 0.0f));

    BASS_ChannelPlay(bassStream, FALSE);

    bool musicCubesInitialized = false;

    float lastTime = glfwGetTime();
    float lastFrameTime = lastTime;
    float frameTime = lastTime;

    float fft[1024];
    float avgBass = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float rawDeltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // smooth out deltas
        float alignedDelta = std::round(rawDeltaTime * frameRate) / frameRate;
        frameTime += alignedDelta;
        frameTime = glm::mix(frameTime, currentTime, 0.05f);
        float deltaTime = frameTime - lastFrameTime;
        lastFrameTime = frameTime;

        double musicTimestamp = BASS_ChannelBytes2Seconds(bassStream, BASS_ChannelGetPosition(bassStream, BASS_POS_BYTE)) + 0.1;
        BASS_ChannelSetPosition(bassStreamDecoded, BASS_ChannelSeconds2Bytes(bassStream, musicTimestamp), BASS_POS_BYTE);

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            BASS_ChannelSetPosition(bassStream, BASS_ChannelSeconds2Bytes(bassStream, musicTimestamp - 0.5), BASS_POS_BYTE);
        } else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            BASS_ChannelSetPosition(bassStream, BASS_ChannelSeconds2Bytes(bassStream, musicTimestamp + 0.5), BASS_POS_BYTE);
        }

        animationTime = static_cast<float>(
            (musicTimestamp + BEAT_OFFSET) * BEATS_PER_SECOND
        );

        lightRimBrightnessAnimation.update(animationTime);
        lightRimAnimation.update(animationTime);
        centerCubeOffset.update(animationTime + 0.1f); // clap offset
        centerCubeRotation.update(animationTime);
        lightRimColorAnimation.update(animationTime);
        cameraPosition.update(animationTime);
        cameraFocus.update(animationTime);
        introMusicCubeAnimation.update(animationTime);
        musicCubeAnimation.update(animationTime);
        dragonAnimation.update(animationTime);
        lucyAnimation.update(animationTime);

        dragonGlassMesh.position = dragonAnimation.get();
        lucyGlassMesh.position = lucyAnimation.get();

        if (introMusicCubeAnimation.get() > 0.0f) {
            for (int i = 0; i < NUM_MUSIC_CUBES; i++) {
                musicCubes[i].scale.y = musicCubes[i].maxHeight * introMusicCubeAnimation.get();
                musicCubes[i].emissionColor = glm::vec3(1.0f);
                musicCubes[i].emissionColorBrightness = introMusicCubeAnimation.get() * introMusicCubeAnimation.get();
                pointLights[i].brightness = musicCubes[i].emissionColorBrightness;
                pointLights[i].color = musicCubes[i].emissionColor;
            }
        }

        BASS_ChannelGetData(bassStreamDecoded, fft, BASS_DATA_FFT2048);
        avgBass = (fft[1] * 0.4f + avgBass * 0.6f);
        float bassBrightness = 0.0f;
        if (avgBass > 0.02f) {
            bassBrightness = 3.0f * avgBass / 0.13f;
        } else {
            avgBass = 0.0f;
            bassBrightness = 0.0f;
        }

        lightRimObject.emissionColorBrightness = lightRimBrightnessAnimation.get();
        lightRimObject.emissionColor = lightRimColorAnimation.get() * lightRimBrightnessAnimation.get();

        if (musicCubeAnimation.get()) {
            if (!musicCubesInitialized) {
                musicCubesInitialized = true;
                for (int i = 0; i < NUM_MUSIC_CUBES; i++) {
                    glm::vec3 color = glm::rgbColor(glm::vec3((360.0f / 36.0f) * i, 0.9f, 1.0f));
                    musicCubes[i].emissionColor = color;
                    pointLights[i].color = color;
                    musicCubes[i].scale = { 1, musicCubes[i].maxHeight, 1 };
                }
            }
            for (int i = 0; i < NUM_MUSIC_CUBES; i++) {
                if (i < NUM_MUSIC_CUBES * (avgBass / 0.13f)) {
                    musicCubes[i].emissionColorBrightness = bassBrightness;
                    glm::vec3 particleSpawnPos = musicCubes[i].position + glm::vec3(0.0f, 0.1f, 0.0f);
                    array<Particle, 10> newParticles;
                    for (unsigned int k = 0; k < 10; k++) {
                        newParticles[k] = {
                            particleSpawnPos,
                            glm::normalize(glm::quat(
                                normalFloats(random),
                                normalFloats(random),
                                normalFloats(random),
                                normalFloats(random)
                            )),
                            glm::normalize(-particleSpawnPos) + glm::vec3(
                                normalFloats(random) * 1.0f,
                                -0.03f,
                                normalFloats(random) * 1.0f
                            ) * 3.0f
                        };
                    }
                    particles.add(&*newParticles.begin(), &*newParticles.end());

                    if (musicCubes[i].scale.y < musicCubes[i].maxHeight) {
                        musicCubes[i].scale.y *= 1.15f;
                    }
                } else {
                    if (musicCubes[i].scale.y > musicCubes[i].minHeight) {
                        musicCubes[i].scale.y *= 0.97f;
                    }
                    musicCubes[i].emissionColorBrightness *= 0.7f;
                }
            }
            for (int i = 0; i < pointLights.size(); i++) {
                if (i < pointLights.size() * (avgBass / 0.13f)) {
                    pointLights[i].brightness = bassBrightness;
                } else {
                    pointLights[i].brightness *= 0.7f;
                }
            }
        } else {
            musicCubesInitialized = false;
        }

        lightRimObject.setModelMatrix(lightRimAnimation.get().to_matrix());

        mat4 centerCubeBase =
            Placement({0, 2, 0}, {0.5, 0.5, 0.5}, {}).to_matrix();

        for (int i = 0; i < 8; i++) {
            vec3 offset{
                i & 1 ? 1 : -1,
                i & 2 ? 1 : -1,
                i & 4 ? 1 : -1,
            };
            mat4 rotation{1};
            if (i & 1) {
                rotation =
                    rotate(rotation, centerCubeRotation.get().x, {1, 0, 0});
            }
            if (i & 2) {
                rotation =
                    rotate(rotation, centerCubeRotation.get().y, {0, 1, 0});
            }
            if (i & 4) {
                rotation =
                    rotate(rotation, centerCubeRotation.get().z, {0, 0, 1});
            }

            centerCubes[i].setModelMatrix(
                centerCubeBase * rotation *
                translate(mat4(1), offset * centerCubeOffset.get())
            );
        }

        glm::mat4 viewMatrix;
        if (useAnimatedCamera) {
            viewMatrix = glm::lookAt(
                glm::mix(cameraPosition.get(), cameraFocus.get(), -0.02),
                cameraFocus.get(),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
            focus =
                glm::length(cameraPosition.get() - cameraFocus.get()) * 1.02f;
        } else {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                camera.pos += camera.front * CAMERA_SPEED * deltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                camera.pos -= camera.front * CAMERA_SPEED * deltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                camera.pos -= camera.right * CAMERA_SPEED * deltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                camera.pos += camera.right * CAMERA_SPEED * deltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                camera.pos += glm::vec3(0.0f, 1.0f, 0.0f) * CAMERA_SPEED * deltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                camera.pos -= glm::vec3(0.0f, 1.0f, 0.0f) * CAMERA_SPEED * deltaTime;
            }
            camera.update();
            //camera.front += glm::vec3(normalFloats(random), normalFloats(random), normalFloats(random)) * avgBass * 0.05f;
            viewMatrix = camera.getViewMatrix();
            cameraPosition.reset();
            cameraFocus.reset();
        }

        particleUpdateShader.use();
        particleUpdateShader.setFloat("delta", deltaTime);
        glDispatchCompute(particles.particleCount, 1, 1);

        glm::mat4 inverseProjectionMatrix = glm::inverse(projectionMatrix);

        glBindFramebuffer(GL_FRAMEBUFFER, cubemapFramebuffer);
        glViewport(0, 0, 256, 256);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        environmentShader.use();
        environmentShader.setMatrix4("view", glm::mat4(1.0f));
        environmentScene.draw(environmentShader);

        glBindFramebuffer(GL_FRAMEBUFFER, blurredEnvironment);
        glViewport(0, 0, 16, 16);
        blurCube.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, environmentColor);
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, nullptr);

        // render geometry, except refractive geometry, to gBuffer
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gBufferShader.use();
        gBufferShader.setMatrix4("model", glm::mat4(1.0f));
        gBufferShader.setMatrix4("view", viewMatrix);
        gBufferShader.setMatrix4("projection", projectionMatrix);
        for (int i = 0; i < pointLights.size(); i++) {
            gBufferShader.setVector3f("pointLights[" + std::to_string(i) + "].pos", glm::vec3(viewMatrix * glm::vec4(pointLights[i].pos, 1.0f)));
            gBufferShader.setVector3f("pointLights[" + std::to_string(i) + "].color", pointLights[i].color);
            gBufferShader.setFloat("pointLights[" + std::to_string(i) + "].brightness", pointLights[i].brightness);
        }

        mainScene.draw(gBufferShader);

        particleShader.use();
        particleShader.setMatrix4("view", viewMatrix);
        particleShader.setMatrix4("projection", projectionMatrix);
        for (int i = 0; i < pointLights.size(); i++) {
            particleShader.setVector3f("pointLights[" + std::to_string(i) + "].pos", glm::vec3(viewMatrix * glm::vec4(pointLights[i].pos, 1.0f)));
            particleShader.setVector3f("pointLights[" + std::to_string(i) + "].color", pointLights[i].color);
            particleShader.setFloat("pointLights[" + std::to_string(i) + "].brightness", pointLights[i].brightness);
        }
        particles.draw(particleShader);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gBufferRefractive);
        glBlitFramebuffer(
            0, 0, windowWidth, windowHeight,
            0, 0, windowWidth, windowHeight,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );

        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lowResFramebuffer);
        glBlitFramebuffer(
            0, 0, windowWidth, windowHeight,
            0, 0, windowWidth / 4, windowHeight / 4,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );

        // render refractive geometry to gBufferRefractive
        glBindFramebuffer(GL_FRAMEBUFFER, gBufferRefractive);
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT);
        gBufferRefractiveShader.use();
        gBufferRefractiveShader.setMatrix4("model", glm::mat4(1.0f));
        gBufferRefractiveShader.setMatrix4("view", viewMatrix);
        gBufferRefractiveShader.setMatrix4("projection", projectionMatrix);
        for (int i = 0; i < pointLights.size(); i++) {
            gBufferRefractiveShader.setVector3f("pointLights[" + std::to_string(i) + "].pos", glm::vec3(viewMatrix * glm::vec4(pointLights[i].pos, 1.0f)));
            gBufferRefractiveShader.setVector3f("pointLights[" + std::to_string(i) + "].color", pointLights[i].color);
            gBufferRefractiveShader.setFloat("pointLights[" + std::to_string(i) + "].brightness", pointLights[i].brightness);
        }

        mainScene.drawGlassObjects(gBufferRefractiveShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // render depth peeled refractive geometry to gBufferLayer2
        glBindFramebuffer(GL_FRAMEBUFFER, gBufferLayer2);
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        gBufferLayer2Shader.use();
        gBufferLayer2Shader.setMatrix4("model", glm::mat4(1.0f));
        gBufferLayer2Shader.setMatrix4("view", viewMatrix);
        gBufferLayer2Shader.setMatrix4("projection", projectionMatrix);

        mainScene.drawGlassObjects(gBufferLayer2Shader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCullFace(GL_BACK);

        glBindVertexArray(screenQuadVAO);

        ssdoPass.shader.use();
        ssdoPass.shader.setMatrix4("view", viewMatrix);
        ssdoPass.shader.setMatrix4("projection", projectionMatrix);
        ssdoPass.shader.setMatrix4(
            "inverseProjection", inverseProjectionMatrix
        );
        ssdoPass.shader.setFloat("environmentBrightness", lightRimObject.emissionColorBrightness);
        ssdoPass.render();

        blurSSDOHorizontal.shader.use();
        blurSSDOHorizontal.shader.setMatrix4(
            "inverseProjection", inverseProjectionMatrix
        );
        blurSSDOHorizontal.render();

        blurSSDOVertical.shader.use();
        blurSSDOVertical.shader.setMatrix4(
            "inverseProjection", inverseProjectionMatrix
        );
        blurSSDOVertical.render();

        refractivePass.shader.use();
        refractivePass.shader.setMatrix4("projection", projectionMatrix);
        refractivePass.render();

        ssrPass.shader.use();
        ssrPass.shader.setMatrix4("view", viewMatrix);
        ssrPass.shader.setMatrix4("projection", projectionMatrix);
        ssrPass.render();

        const float aperture = 0.1f;
        const float focalLength = 0.2f;
        float infinityRadius =
            aperture * focalLength / (focus - focalLength) *
            windowWidth / std::tan(fov * 0.5f) / 16;

        dofCocPass.shader.use();
        dofCocPass.shader.setFloat("focus", focus);
        dofCocPass.shader.setFloat("infinityRadius", infinityRadius);
        dofCocPass.render();
        dofCoarsePass.render();
        dofFinePass.render();

        // copy aliased image to anti aliased buffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, dofFinePass.framebuffer);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampleBuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(
            0, 0, windowWidth, windowHeight,
            0, 0, windowWidth, windowHeight,
            GL_COLOR_BUFFER_BIT, GL_NEAREST
        );

        glBindFramebuffer(GL_FRAMEBUFFER, multisampleBuffer);
        glClear(GL_DEPTH_BUFFER_BIT);
        antiAliasingProgram.use();
        antiAliasingProgram.setMatrix4("view", viewMatrix);
        antiAliasingProgram.setMatrix4("projection", projectionMatrix);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, dofTexture);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, gPrimitiveID);
        mainScene.draw(antiAliasingProgram);
        //particles.draw(antiAliasingProgram);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleBuffer);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, filterFramebuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(
            0, 0, windowWidth, windowHeight,
            0, 0, windowWidth, windowHeight,
            GL_COLOR_BUFFER_BIT, GL_NEAREST
        );

        glBindVertexArray(screenQuadVAO);

        bloomHorizontalPass.render();
        bloomVerticalPass.render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // need to clear because default FB has a depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        composePass.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    BASS_Free();
    glfwTerminate();
    return 0;
}

void mouseCallback(GLFWwindow*, double xPos, double yPos) {
    camera.processMouseMovement(xPos, yPos);
}

void scrollCallback(GLFWwindow*, double, double yoffset) {
    focus += static_cast<float>(yoffset) * 1.0f;
    focus = std::min(std::max(focus, 0.5f), 50.0f);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        muteSong = !muteSong;
        if (muteSong) {
            BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 0);
        } else {
            BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 10000);
        }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        useAnimatedCamera = !useAnimatedCamera;
    }

    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        std::cout << "Animation time: " << animationTime << " Camera position: " << camera.pos.x << ", " << camera.pos.y << ", " << camera.pos.z << std::endl;
    }
}

GLuint generateTexture() {
    return generateTexture(windowWidth, windowHeight);
}

GLuint generateTexture(int width, int height) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

    return texture;
}

GLFWwindow *initGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 0); // turn off multisample for framebuffer 0
    glfwSwapInterval(1);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    windowWidth = mode->width;
    windowHeight = mode->height;
    frameRate = mode->refreshRate;
    GLFWwindow *window;

    if (fullscreen) {
        window = glfwCreateWindow(
            windowWidth, windowHeight, "Brechpunkt", monitor, nullptr
        );
    } else {
        windowWidth = DEFAULT_WIDTH;
        windowHeight = DEFAULT_HEIGHT;

        window = glfwCreateWindow(
            windowWidth, windowHeight, "Brechpunkt", nullptr, nullptr
        );
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return window;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return window;
}

bool initGLEW() {
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    return true;
}

GLuint getScreenQuadVAO() {
    GLfloat quadVertices[] = {
        // Positions        // Texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f
    };

    // create and bind VAO
    GLuint screenQuadVAO;
    glGenVertexArrays(1, &screenQuadVAO);
    glBindVertexArray(screenQuadVAO);

    // create and bind VBO
    GLuint quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

    // copy vertices array to VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // set vertex attribute pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

    // unbind VAO
    glBindVertexArray(0);

    return screenQuadVAO;
}

void drawScreenQuad(GLuint screenQuadVAO) {
    glBindVertexArray(screenQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
