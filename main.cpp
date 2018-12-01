#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <vector>
#include <random>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/color_space.hpp>

#include "RessourceManager.h"
#include "Mesh.h"
#include "Animation.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Effect.h"
#include "ParticleSystem.h"
#include "Scene.h"

using namespace std;

struct PointLight {
    glm::vec3 pos;
    glm::vec3 color;
    float constantTerm;
    float linearTerm;
    float quadraticTerm;

    PointLight(glm::vec3 pos, glm::vec3 color, float constantTerm, float linearTerm, float quadraticTerm) :
        pos(pos),
        color(color),
        constantTerm(constantTerm),
        linearTerm(linearTerm),
        quadraticTerm(quadraticTerm)
    {
    }
};

static bool debug_flag = false;

const int DEFAULT_WIDTH = 1280;
const int DEFAULT_HEIGHT = 720;
const float CAMERA_SPEED = 10.0f;

static int windowWidth = 0, windowHeight = 0;
static float frameRate;

static vector<PointLight> pointLights;
static Camera camera;
static float focus = 4;
static Program gBufferShader, particleShader, composeShader, ssdoShader;
static Program environmentShader;
static Program particleUpdateShader;
static bool useAnimatedCamera = true;

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
// with parameters p1 = 2, p2 = 7
const glm::vec2 HALTON_POINTS[64] = {
    glm::vec2(0, 0),
    glm::vec2(0.5, 0.142857),
    glm::vec2(0.25, 0.285714),
    glm::vec2(0.75, 0.428571),
    glm::vec2(0.125, 0.571429),
    glm::vec2(0.625, 0.714286),
    glm::vec2(0.375, 0.857143),
    glm::vec2(0.875, 0.0204082),
    glm::vec2(0.0625, 0.163265),
    glm::vec2(0.5625, 0.306122),
    glm::vec2(0.3125, 0.44898),
    glm::vec2(0.8125, 0.591837),
    glm::vec2(0.1875, 0.734694),
    glm::vec2(0.6875, 0.877551),
    glm::vec2(0.4375, 0.0408163),
    glm::vec2(0.9375, 0.183673),
    glm::vec2(0.03125, 0.326531),
    glm::vec2(0.53125, 0.469388),
    glm::vec2(0.28125, 0.612245),
    glm::vec2(0.78125, 0.755102),
    glm::vec2(0.15625, 0.897959),
    glm::vec2(0.65625, 0.0612245),
    glm::vec2(0.40625, 0.204082),
    glm::vec2(0.90625, 0.346939),
    glm::vec2(0.09375, 0.489796),
    glm::vec2(0.59375, 0.632653),
    glm::vec2(0.34375, 0.77551),
    glm::vec2(0.84375, 0.918367),
    glm::vec2(0.21875, 0.0816327),
    glm::vec2(0.71875, 0.22449),
    glm::vec2(0.46875, 0.367347),
    glm::vec2(0.96875, 0.510204),
    glm::vec2(0.015625, 0.653061),
    glm::vec2(0.515625, 0.795918),
    glm::vec2(0.265625, 0.938776),
    glm::vec2(0.765625, 0.102041),
    glm::vec2(0.140625, 0.244898),
    glm::vec2(0.640625, 0.387755),
    glm::vec2(0.390625, 0.530612),
    glm::vec2(0.890625, 0.673469),
    glm::vec2(0.078125, 0.816327),
    glm::vec2(0.578125, 0.959184),
    glm::vec2(0.328125, 0.122449),
    glm::vec2(0.828125, 0.265306),
    glm::vec2(0.203125, 0.408163),
    glm::vec2(0.703125, 0.55102),
    glm::vec2(0.453125, 0.693878),
    glm::vec2(0.953125, 0.836735),
    glm::vec2(0.046875, 0.979592),
    glm::vec2(0.546875, 0.00291545),
    glm::vec2(0.296875, 0.145773),
    glm::vec2(0.796875, 0.28863),
    glm::vec2(0.171875, 0.431487),
    glm::vec2(0.671875, 0.574344),
    glm::vec2(0.421875, 0.717201),
    glm::vec2(0.921875, 0.860058),
    glm::vec2(0.109375, 0.0233236),
    glm::vec2(0.609375, 0.166181),
    glm::vec2(0.359375, 0.309038),
    glm::vec2(0.859375, 0.451895),
    glm::vec2(0.234375, 0.594752),
    glm::vec2(0.734375, 0.737609),
    glm::vec2(0.484375, 0.880467),
    glm::vec2(0.984375, 0.0437318)
};

int main(int argc, const char** argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], "--debug") == 0) {
            debug_flag = true;
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

    std::default_random_engine random;

    GLuint gColor, gWorldPos, gNormal, gReflection, gEmission, gDepth;
    GLuint gBuffer = generateFramebufferMultisample(
        windowWidth, windowHeight, 1, {
            {GL_COLOR_ATTACHMENT0, gColor, GL_RGB16F},
            {GL_COLOR_ATTACHMENT1, gWorldPos, GL_RGB16F},
            {GL_COLOR_ATTACHMENT2, gNormal, GL_RGB16F},
            {GL_COLOR_ATTACHMENT3, gReflection, GL_RGB16F},
            {GL_COLOR_ATTACHMENT4, gEmission, GL_RGB16F},
            {GL_DEPTH_ATTACHMENT, gDepth, GL_DEPTH_COMPONENT16},
        }, {
        }
    );

    GLuint filteredBuffer;
    GLuint filterFramebuffer = generateFramebuffer(
        windowWidth, windowHeight, {
            {GL_COLOR_ATTACHMENT0, filteredBuffer, GL_RGB16F}
        }, {}
    );

    glEnable(GL_DEPTH_TEST);
    GLuint environmentColor, environmentDepth;
    GLuint cubemapFramebuffer = generateFramebuffer(
        256, 256, GL_TEXTURE_CUBE_MAP, {
            {GL_COLOR_ATTACHMENT0, environmentColor, GL_RGB16F},
            {GL_DEPTH_ATTACHMENT, environmentDepth, GL_DEPTH_COMPONENT16},
        }, {
        }
    );

    GLuint multisampleColor, multisampleDepth;
    GLuint multisampleBuffer = generateFramebufferMultisample(
        windowWidth, windowHeight, 8, {
            {GL_COLOR_ATTACHMENT0, multisampleColor, GL_RGB16F},
        }, {
            {GL_DEPTH_ATTACHMENT, multisampleDepth, GL_DEPTH_COMPONENT16},
        }
    );

    GLuint screenQuadVAO = getScreenQuadVAO();

    Scene environmentScene, mainScene;

    MeshInfo musicCubeMeshInfo("scenes/scene3/", "MusicCube.obj");
    MeshInfo floorMeshInfo("scenes/scene3/", "Floor.obj");
    MeshInfo centerCubeMeshInfo("scenes/scene3/", "CenterCube.obj");
    MeshInfo lightRimInfo("scenes/scene3/", "LightRim.obj");

    Mesh lightRimObject = Mesh(
        lightRimInfo, glm::translate(
            glm::mat4(1.0f), glm::vec3(0, 0, 0)
        ), {}, {2, 2, 2}
    );

    mainScene.objects.insert(&lightRimObject);
    environmentScene.objects.insert(&lightRimObject);

    Mesh floorObject = Mesh(
        floorMeshInfo,
        glm::mat4(1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.0f)
    );

    mainScene.objects.insert(&floorObject);

    ParticleSystem particles(10000, 3, 4);

    std::normal_distribution<float> normalFloats;
    for (int i = 0; i < 10000; i++) {
        particles.add(
            {
                normalFloats(random) * 5,
                0.1,
                normalFloats(random) * 5
            }, glm::normalize(glm::quat(
                normalFloats(random),
                normalFloats(random),
                normalFloats(random),
                normalFloats(random)
            ))
        );
    }

    Mesh musicCubes[36];

    const float lightFloorOffset = 2.0f;
    for (int i = 0; i < 36; i++) {
        auto pos = glm::vec3(0.0f, 0.0f, -20.0f);
        auto model = glm::mat4(1.0f);
        model = glm::rotate(
            model, glm::radians(i * 10.0f), { 0, 1, 0 }
        );
        model = glm::translate(model, pos);

        auto color = glm::rgbColor(glm::vec3((360.0f / 36.0f) * i, 0.9f, 1.0f));

        musicCubes[i] = Mesh(
            musicCubeMeshInfo,
            model,
            glm::vec3(0.0f),
    	    color
        );

        mainScene.objects.insert(&musicCubes[i]);

        pointLights.push_back(PointLight(
            model * glm::vec4(0.0f, lightFloorOffset, 0.0f, 1.0f),
            color,
            1.0f,
            0.07f,
            0.20f
        ));
    }

    auto centerCubeModel =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
    Mesh centerCubeObject(
        centerCubeMeshInfo,
        centerCubeModel,
        glm::vec3(1.0f, 1.0f, 1.0f) * 2.0f,
        glm::vec3(0.0f)
    );

    mainScene.objects.insert(&centerCubeObject);

    float near = 0.5f;
    float far = 100.0f;
    float fov = glm::radians(45.0f);
    glm::mat4 projectionMatrix = glm::perspective(
        fov, (float)windowWidth / (float)windowHeight, near, far
    );

    Animation<glm::vec3> cameraPosition{
        {0, {0, 60.0, -2.4}, HandleType::STOP},
        {18, {-3.7, 0.8, -3.4}, HandleType::STOP},
        {20, {-3.7, 0.8, -3.4}, HandleType::STOP},

        {10, {8.5, 1.5, -10.0}, HandleType::SMOOTH_OUT},
        {30, {8.5, 1.5, 0.0}, HandleType::SMOOTH_IN},

        {30, {4.9, 2.8, -1.1}, HandleType::STOP},
        {40, {4.9, 2.8, -1.1}, HandleType::STOP},
    };

    Animation<glm::vec3> cameraFocus{
        {0, {0, 2, 0}, HandleType::STOP},
        {20, {-0.8, 2, -1}, HandleType::STOP},

        {10, {4, 0.0, -8.0}, HandleType::SMOOTH_OUT},
        {30, {4, 0.0, -2.0}, HandleType::SMOOTH_IN},

        {30, {-19, 2.8, -1.1}, HandleType::STOP},
        {34, {-19, 2.8, -1.1}, HandleType::STOP},
        {36, {0.8, 2.8, -1.1}, HandleType::STOP},
        {40, {0.8, 2.8, -1.1}, HandleType::STOP},
    };

    gBufferShader = Program("shaders/gBuffer.vert", "shaders/gBuffer.frag");
    environmentShader = Program(
        "shaders/environment.vert", "shaders/environment.geom",
        "shaders/environment.frag"
    );

    particleShader = Program("shaders/particle.vert", "shaders/particle.frag");

    particleUpdateShader = Program("shaders/particleUpdate.comp");
    particleUpdateShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particles.instanceVbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particles.physicVbo);

	GLuint ssdoUnblurredTexture, ssdoTexture, noiseTexture;
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	glm::vec3 randomValues[16];
	for (int k = 0; k < 16; k++) {
		randomValues[k] = glm::vec3(
            randomFloats(random) * 2.0f - 1.0f,
            randomFloats(random) * 2.0f - 1.0f,
			0.0f
		);
	}
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &randomValues[0]);
    auto ssdoPass = Effect(
        "shaders/ssdo.frag", windowWidth, windowHeight,
        {
            {"noiseTex", GL_TEXTURE_2D, noiseTexture},
            {"environmentColor", GL_TEXTURE_CUBE_MAP, environmentColor},
            {"gColorTex", GL_TEXTURE_2D_MULTISAMPLE, gColor},
            {"gNormalTex", GL_TEXTURE_2D_MULTISAMPLE, gNormal},
            {"gWorldPosTex", GL_TEXTURE_2D_MULTISAMPLE, gWorldPos},
            {"gEmissionTex", GL_TEXTURE_2D_MULTISAMPLE, gEmission}
        },
        { {"color", ssdoUnblurredTexture, GL_RGB16F} }
    );

    GLuint ssdoBlurHorizontalTexture;
    auto blurSSDOHorizontal = Effect(
        "shaders/blurSSDOHorizontal.frag", windowWidth, windowHeight,
        {
            {"colorTex", GL_TEXTURE_2D, ssdoUnblurredTexture},
            {"gNormalTex", GL_TEXTURE_2D_MULTISAMPLE, gNormal}
        },
        { {"color", ssdoBlurHorizontalTexture, GL_RGB16F} }
    );
    auto blurSSDOVertical = Effect(
        "shaders/blurSSDOVertical.frag", windowWidth, windowHeight,
        {
            {"colorTex", GL_TEXTURE_2D, ssdoBlurHorizontalTexture},
            {"gNormalTex", GL_TEXTURE_2D_MULTISAMPLE, gNormal}
        },
        { {"color", ssdoTexture, GL_RGB16F} }
    );

    Program antiAliasingProgram(
        "shaders/antiAlias.vert", "shaders/antiAlias.frag"
    );
    antiAliasingProgram.use();
    glUniform1i(
        glGetUniformLocation(antiAliasingProgram.program, "colorTex"), 0
    );
    glUniform1i(
        glGetUniformLocation(antiAliasingProgram.program, "gDepthTex"), 1
    );
    glUniform1i(
        glGetUniformLocation(antiAliasingProgram.program, "gNormalTex"), 2
    );

    GLuint bloomHorizontalTexture, bloomTexture;
    auto bloomHorizontalPass = Effect(
        "shaders/bloomHorizontal.frag", windowWidth / 2, windowHeight / 2,
        { {"colorTex", GL_TEXTURE_2D_MULTISAMPLE, multisampleColor} },
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
            {"depthTex", GL_TEXTURE_2D_MULTISAMPLE, gDepth},
        }, {
            {"coc", dofCocTexture, GL_R8_SNORM},
        }
    );
    auto dofCoarsePass = Effect(
        "shaders/dofCoarse.frag", windowWidth, windowHeight, {
            {"colorTex", GL_TEXTURE_2D, ssdoTexture},
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

    float hemisphereSamples[192];
    for (int i = 0; i < 64; i++) {
        glm::vec3 hemisphereSample = getHemisphereSample(HALTON_POINTS[i]) * randomFloats(random);
        hemisphereSamples[i * 3] = hemisphereSample.x;
        hemisphereSamples[i * 3 + 1] = hemisphereSample.y;
        hemisphereSamples[i * 3 + 2] = hemisphereSample.z;
    }

    ssdoPass.shader.use();
    glUniform3fv(glGetUniformLocation(ssdoPass.shader.program, "hemisphereSamples"), 64, &hemisphereSamples[0]);

    float lastTime = glfwGetTime();
    float lastFrameTime = lastTime;
    float frameTime = lastTime;

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

        glm::mat4 viewMatrix;
        if (useAnimatedCamera) {
            cameraPosition.update(deltaTime);
            cameraFocus.update(deltaTime);
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

            viewMatrix = camera.getViewMatrix();
            cameraPosition.reset();
        }

        particleUpdateShader.use();
        particleUpdateShader.setFloat("delta", deltaTime);
        glDispatchCompute(particles.particleCount, 1, 1);

        glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

        glBindFramebuffer(GL_FRAMEBUFFER, cubemapFramebuffer);
        //glDepthFunc(GL_LESS);
        glViewport(0, 0, 256, 256);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        environmentShader.use();
        environmentShader.setMatrix4("view", viewMatrix);
        environmentScene.draw(environmentShader);

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
        }

        mainScene.draw(gBufferShader);

        particleShader.use();
        particleShader.setMatrix4("view", viewMatrix);
        particleShader.setMatrix4("projection", projectionMatrix);
        for (int i = 0; i < pointLights.size(); i++) {
            particleShader.setVector3f("pointLights[" + std::to_string(i) + "].pos", glm::vec3(viewMatrix * glm::vec4(pointLights[i].pos, 1.0f)));
            particleShader.setVector3f("pointLights[" + std::to_string(i) + "].color", pointLights[i].color);
        }
        particles.draw(particleShader);

        glBindVertexArray(screenQuadVAO);

        ssdoPass.shader.use();
        ssdoPass.shader.setMatrix4("view", viewMatrix);
        ssdoPass.shader.setMatrix4("projection", projectionMatrix);
        ssdoPass.render();

        blurSSDOHorizontal.render();
        blurSSDOVertical.render();

        const float aperture = 0.05f;
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

        glBindFramebuffer(GL_FRAMEBUFFER, multisampleBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glDepthFunc(GL_EQUAL);
        antiAliasingProgram.use();
        antiAliasingProgram.setMatrix4("view", viewMatrix);
        antiAliasingProgram.setMatrix4("projection", projectionMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, dofTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gDepth);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gNormal);
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
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        useAnimatedCamera = !useAnimatedCamera;
    }

    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        std::cout << "Camera position: " << camera.pos.x << ", " << camera.pos.y << ", " << camera.pos.z << std::endl;
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

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    windowWidth = mode->width;
    windowHeight = mode->height;
    frameRate = mode->refreshRate;
    GLFWwindow *window;

    if (debug_flag) {
        windowWidth = DEFAULT_WIDTH;
        windowHeight = DEFAULT_HEIGHT;

        window = glfwCreateWindow(
            windowWidth, windowHeight, "Brechpunkt", nullptr, nullptr
        );
    } else {
        window = glfwCreateWindow(
            windowWidth, windowHeight, "Brechpunkt", monitor, nullptr
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
