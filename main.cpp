#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <vector>
#include <random>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "RessourceManager.h"
#include "Mesh.h"
#include "Animation.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Effect.h"

using namespace std;

enum TextureUnit : GLuint {
    MATERIAL_DIFFUSE_TEXTURE_UNIT,
    MATERIAL_REFLECTION_TEXTURE_UNIT,
    MATERIAL_NORMAL_TEXTURE_UNIT,
    VIEWPOS_TEXTURE_UNIT,

    NORMAL_TEXTURE_UNIT,
    SSDO_TEXTURE_UNIT,
    SSDO_NOISE_TEXTURE_UNIT,
	SSDO_SAMPLES_TEXTURE,
    SSDO_HORIZONTAL_BLUR_TEXTURE_UNIT,
    SSDO_FINAL_TEXTURE_UNIT,

    LIGHT_BOUNCE_TEXTURE_UNIT,
    LIGHT_BOUNCE_HORIZONTAL_BLUR_TEXTURE_UNIT,
    LIGHT_BOUNCE_FINAL_TEXTURE_UNIT,

    COLOR_TEXTURE_UNIT,
    COLOR_FILTERED_TEXTURE_UNIT,

    BLOOM_HORIZONTAL_TEXTURE_UNIT,
    BLOOM_FINAL_TEXTURE_UNIT,
};

static bool debug_flag = false;

const int DEFAULT_WIDTH = 1920;
const int DEFAULT_HEIGHT = 1080;
const float CAMERA_SPEED = 10.0f;

static int window_width = 0, window_height = 0;

static Camera camera;
static Shader gBufferShader;
static Shader composeShader;
static Shader ssdoShader;
static vector<Mesh> meshes;
static float deltaTime;
static bool useAnimatedCamera = false;
GLuint blurFBO0, blurFBO1;
GLuint blurBuffer0, blurBuffer1;

GLuint loadTexture(std::string textureFileName);
void loadObj(std::string basedir, std::string objFileName);
GLFWwindow *initGLFW();
bool initGLEW();
GLuint getScreenQuadVAO();
GLuint generateTexture();
GLuint generateTexture(int width, int height);
GLuint generateNoiseTexture();
void drawScreenQuad(GLuint screenQuadVAO);
void mouseCallback(GLFWwindow* window, double xPos, double yPos);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLuint blur(int width, int height, GLuint texture, Shader &shader, int kernelSize, GLuint screenQuadVAO, GLuint gNormal);

glm::vec3 getHemisphereSample(glm::vec2 u) {
    // source: "Physically Based Rendering: From Theory to Implementation" [Pharr and Humphreys, 2016]
    float r = std::sqrt(u.x);
    float phi = 2.0f * 3.1415926f * u.y;
    return glm::vec3(
        r * std::cosf(phi),
        r * std::sinf(phi),
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
    glfwSetKeyCallback(window, keyCallback);

    GLuint gColor, gWorldPos, gNormal, gReflection, depthRBO;
    GLuint gBuffer = generateFramebufferMultisample(
        window_width, window_height, 4, {
            {GL_COLOR_ATTACHMENT0, gColor, GL_RGB16F},
            {GL_COLOR_ATTACHMENT1, gWorldPos, GL_RGB16F},
            {GL_COLOR_ATTACHMENT2, gNormal, GL_RGB16F},
            {GL_COLOR_ATTACHMENT3, gReflection, GL_RGB16F},
        }, {
            {GL_DEPTH_ATTACHMENT, depthRBO, GL_DEPTH_COMPONENT},
        }
        );
    GLuint gColorFiltered;
    GLuint filterFramebuffer = generateFramebuffer(
        window_width, window_height, {
            {GL_COLOR_ATTACHMENT0, gColorFiltered, GL_RGB16F}
        }, {}
    );

    glEnable(GL_DEPTH_TEST);

    GLuint screenQuadVAO = getScreenQuadVAO();

    loadObj("scenes/scene1/", "demolevel.obj");

    float near = 0.1f;
    float far = 100.0f;
    glm::mat4 projectionMatrix = glm::perspective(
        glm::radians(45.0f), (float)window_width / (float)window_height, near, far
    );

    Animation<glm::vec3> cameraPosition{
        {0, {0, 0.5, 4}, HandleType::STOP},
        {4, {3, 0.5, 4}},
        {10, {3, 0.5, 7}, HandleType::SMOOTH_IN},
        {10, {0, 0.5, 4}, HandleType::SMOOTH_OUT},
        {14, {3, 0.5, 4}},
        {20, {3, 0.5, 7}, HandleType::STOP},
    };

    gBufferShader = Shader("shaders/gBuffer.vert", "shaders/gBuffer.frag");
    composeShader = Shader("shaders/compose.vert", "shaders/compose.frag");

    auto bloomHorizontalPass = Effect(
        "shaders/bloomHorizontal.frag", window_width / 2, window_height / 2,
        { {"colorTex", SSDO_FINAL_TEXTURE_UNIT} },
        { {"color", BLOOM_HORIZONTAL_TEXTURE_UNIT, GL_RGB16F} }
    );

    auto bloomVerticalPass = Effect(
        "shaders/bloomVertical.frag", window_width / 2, window_height / 2,
        { {"colorTex", BLOOM_HORIZONTAL_TEXTURE_UNIT} },
        { {"color", BLOOM_FINAL_TEXTURE_UNIT, GL_RGB16F} }
    );

    auto ssdoPass = Effect(
        "shaders/ssdo.frag", window_width, window_height,
        {
          {"gColorTex", COLOR_FILTERED_TEXTURE_UNIT},
          {"gNormalTex", NORMAL_TEXTURE_UNIT},
          {"gWorldPosTex", VIEWPOS_TEXTURE_UNIT}
        },
        { {"color", SSDO_TEXTURE_UNIT, GL_RGB16F} }
    );
    auto blurSSDOHorizontal = Effect(
        "shaders/blurSSDOHorizontal.frag", window_width, window_height,
        {
            {"colorTex", SSDO_TEXTURE_UNIT},
            {"gNormalTex", NORMAL_TEXTURE_UNIT}
        },
        { {"color", SSDO_HORIZONTAL_BLUR_TEXTURE_UNIT, GL_RGB16F} }
    );
    auto blurSSDOVertical = Effect(
        "shaders/blurSSDOVertical.frag", window_width, window_height,
        {
            {"colorTex", SSDO_HORIZONTAL_BLUR_TEXTURE_UNIT},
            {"gNormalTex", NORMAL_TEXTURE_UNIT}
        },
        { {"color", SSDO_FINAL_TEXTURE_UNIT, GL_RGB16F} }
    );

    auto lightBouncePass = Effect(
        "shaders/lightBounce.frag", window_width / 2, window_height / 2,
        {
          {"gColorTex", COLOR_FILTERED_TEXTURE_UNIT},
          {"gNormalTex", NORMAL_TEXTURE_UNIT},
          {"gWorldPosTex", VIEWPOS_TEXTURE_UNIT}
        },
        { {"color", LIGHT_BOUNCE_TEXTURE_UNIT, GL_RGB16F} }
    );
    auto blurLightBounceHorizontalPass = Effect(
        "shaders/blurLightBounceHorizontal.frag", window_width / 2, window_height / 2,
        { {"colorTex", LIGHT_BOUNCE_TEXTURE_UNIT} },
        { {"color", LIGHT_BOUNCE_HORIZONTAL_BLUR_TEXTURE_UNIT, GL_RGB16F} }
    );
    auto blurLightBounceVerticalPass = Effect(
        "shaders/blurLightBounceVertical.frag", window_width / 2, window_height / 2,
        { {"colorTex", LIGHT_BOUNCE_HORIZONTAL_BLUR_TEXTURE_UNIT} },
        { {"color", LIGHT_BOUNCE_FINAL_TEXTURE_UNIT, GL_RGB16F} }
    );

    composeShader.use();
    glUniform1i(
        glGetUniformLocation(composeShader.program, "colorTex"),
        COLOR_TEXTURE_UNIT
    );
    glUniform1i(
        glGetUniformLocation(composeShader.program, "bloomTex"),
        BLOOM_FINAL_TEXTURE_UNIT
    );
    glUniform1i(
        glGetUniformLocation(composeShader.program, "occlusionTex"),
        SSDO_FINAL_TEXTURE_UNIT
    );
    glUniform1i(
        glGetUniformLocation(composeShader.program, "lightBounceTex"),
        LIGHT_BOUNCE_FINAL_TEXTURE_UNIT
    );
    glActiveTexture(GL_TEXTURE0 + COLOR_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gColor);
    glActiveTexture(GL_TEXTURE0 + COLOR_FILTERED_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, gColorFiltered);
    glActiveTexture(GL_TEXTURE0 + NORMAL_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gNormal);
    glActiveTexture(GL_TEXTURE0 + VIEWPOS_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gWorldPos);

    camera = Camera(glm::vec3(0.0f, 1.0f, 0.0f));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    glm::vec3 randomValues[16];
    for (int k = 0; k < 16; k++) {
        randomValues[k] = glm::vec3(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f
        );
    }
    GLuint noiseTexture;
    glGenTextures(1, &noiseTexture);
    glActiveTexture(GL_TEXTURE0 + SSDO_NOISE_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &randomValues[0]);

    float hemisphereSamples[192];
    for (int i = 0; i < 64; i++) {
        glm::vec3 hemisphereSample = getHemisphereSample(HALTON_POINTS[i]) * randomFloats(generator);
        hemisphereSamples[i * 3] = hemisphereSample.x;
        hemisphereSamples[i * 3 + 1] = hemisphereSample.y;
        hemisphereSamples[i * 3 + 2] = hemisphereSample.z;
    }

    ssdoPass.shader.use();
    glUniform1i(
        glGetUniformLocation(ssdoPass.shader.program, "noiseTex"),
        SSDO_NOISE_TEXTURE_UNIT
    );
    glUniform3fv(glGetUniformLocation(ssdoPass.shader.program, "hemisphereSamples"), 64, &hemisphereSamples[0]);
    glUniform2f(glGetUniformLocation(ssdoPass.shader.program, "size"), DEFAULT_WIDTH, DEFAULT_HEIGHT);

    lightBouncePass.shader.use();
    glUniform1i(
        glGetUniformLocation(lightBouncePass.shader.program, "noiseTex"),
        SSDO_NOISE_TEXTURE_UNIT
    );
    glUniform3fv(glGetUniformLocation(lightBouncePass.shader.program, "hemisphereSamples"), 64, &hemisphereSamples[0]);
    glUniform2f(glGetUniformLocation(ssdoPass.shader.program, "size"), DEFAULT_WIDTH, DEFAULT_HEIGHT);

    float lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glm::mat4 viewMatrix;
        if (useAnimatedCamera) {
            cameraPosition.update(1.0f / 60); // TODO: get delta from music playback
            viewMatrix = glm::lookAt(
                cameraPosition.get(),
                cameraPosition.get() + glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
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

        // g-buffer
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gBufferShader.use();
        gBufferShader.setMatrix4("view", viewMatrix);
        gBufferShader.setMatrix4("projection", projectionMatrix);

        for (int i = 0; i < meshes.size(); i++) {
            meshes[i].draw(gBufferShader);
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, filterFramebuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(
            0, 0, window_width, window_height,
            0, 0, window_width, window_height,
            GL_COLOR_BUFFER_BIT, GL_NEAREST
        );

        glBindVertexArray(screenQuadVAO);

        ssdoPass.shader.use();
        ssdoPass.shader.setMatrix4("view", viewMatrix);
        ssdoPass.shader.setMatrix4("projection", projectionMatrix);
        ssdoPass.render();
		
        blurSSDOHorizontal.render();
        blurSSDOVertical.render();

        bloomHorizontalPass.render();
        bloomVerticalPass.render();

        lightBouncePass.shader.use();
        lightBouncePass.shader.setMatrix4("view", viewMatrix);
        lightBouncePass.shader.setMatrix4("projection", projectionMatrix);
        lightBouncePass.render();
        blurLightBounceHorizontalPass.render();
        blurLightBounceVerticalPass.render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // need to clear because default FB has a depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        composeShader.use();
        drawScreenQuad(screenQuadVAO);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

GLuint blur(int width, int height, GLuint texture, Shader &shader, int kernelSize, GLuint screenQuadVAO, GLuint gNormal) {
    shader.use();
    shader.setVector2f("size", glm::vec2(width, height));

    // blur horizontally
    glBindFramebuffer(GL_FRAMEBUFFER, blurFBO1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    shader.setVector2f("dir", glm::vec2(1, 0));
    shader.setTexture2D("tex", GL_TEXTURE0, texture, 0);
    shader.setTexture2D("normalTex", GL_TEXTURE1, gNormal, 1);
    shader.setInteger("kernelSize", kernelSize);
    drawScreenQuad(screenQuadVAO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // blur vertically
    glBindFramebuffer(GL_FRAMEBUFFER, blurFBO0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    shader.setVector2f("dir", glm::vec2(0, 1));
    shader.setTexture2D("tex", GL_TEXTURE0, blurBuffer1, 0);
    shader.setTexture2D("normalTex", GL_TEXTURE1, gNormal, 1);
    shader.setInteger("kernelSize", kernelSize);
    drawScreenQuad(screenQuadVAO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return blurBuffer0;
}

void mouseCallback(GLFWwindow* window, double xPos, double yPos) {
    camera.processMouseMovement(xPos, yPos);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        gBufferShader = Shader("shaders/gBuffer.vert", "shaders/gBuffer.frag");
        composeShader = Shader("shaders/compose.vert", "shaders/compose.frag");
        ssdoShader = Shader("shaders/ssdo.vert", "shaders/ssdo.frag");
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

void loadObj(std::string basedir, std::string objFileName) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    tinyobj::LoadObj(&attrib, &shapes, &materials, &err, (basedir + objFileName).c_str(), basedir.c_str());
    if (!err.empty()) {
        std::cout << err << std::endl;
        return;
    }

    for (int i = 0; i < materials.size(); i++) {
        if (RessourceManager::materials.find(materials[i].name) == RessourceManager::materials.end()) {
            RessourceManager::materials[materials[i].name] = materials[i];
            if (materials[i].diffuse_texname.length() > 0 && RessourceManager::textures.find(materials[i].diffuse_texname) == RessourceManager::textures.end()) {
                RessourceManager::textures[materials[i].diffuse_texname] = loadTexture(basedir + materials[i].diffuse_texname);
            }
            if (materials[i].metallic_texname.length() > 0 && RessourceManager::textures.find(materials[i].metallic_texname) == RessourceManager::textures.end()) {
                RessourceManager::textures[materials[i].metallic_texname] = loadTexture(basedir + materials[i].metallic_texname);
            }
            if (materials[i].normal_texname.length() > 0 && RessourceManager::textures.find(materials[i].normal_texname) == RessourceManager::textures.end()) {
                RessourceManager::textures[materials[i].normal_texname] = loadTexture(basedir + materials[i].normal_texname);
            }
        }
    }

    for (int i = 0; i < shapes.size(); i++) {
        vector<float> meshData;

        int indexOffset = 0;
        for (int k = 0; k < shapes[i].mesh.num_face_vertices.size(); k++) {
            unsigned int faceVertices = shapes[i].mesh.num_face_vertices[k];
            for (int m = 0; m < faceVertices; m++) {
                tinyobj::index_t index = shapes[i].mesh.indices[indexOffset + m];

                meshData.push_back(attrib.vertices[3 * index.vertex_index + 0]);
                meshData.push_back(attrib.vertices[3 * index.vertex_index + 1]);
                meshData.push_back(attrib.vertices[3 * index.vertex_index + 2]);

                meshData.push_back(attrib.normals[3 * index.normal_index + 0]);
                meshData.push_back(attrib.normals[3 * index.normal_index + 1]);
                meshData.push_back(attrib.normals[3 * index.normal_index + 2]);

                meshData.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
                meshData.push_back(attrib.texcoords[2 * index.texcoord_index + 1]);
            }
            indexOffset += faceVertices;
        }

        unsigned int VAO;
        unsigned int VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(float), &meshData[0], GL_DYNAMIC_DRAW);

        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // texture coordinate
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        /*
        if (mesh.name == "asdfasdf") {
            add lamp
        }
        */

        meshes.push_back(Mesh(
            VAO,
            materials[shapes[i].mesh.material_ids[0]].name,
            meshData.size() / 8.0f,
            glm::vec3(0.0f)
        ));
    }
}

GLuint loadTexture(std::string textureFileName) {
    int width, height, channels;
    unsigned char *data = stbi_load(textureFileName.c_str(), &width, &height, &channels, 0);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return texture;
}

GLuint generateNoiseTexture() {
    uniform_real_distribution<GLfloat> randomFloats(0.0f, 1.0f);
    default_random_engine randomEngine;
    vector<glm::vec3> noise;
    for (int i = 0; i < 64; i++) {
        noise.push_back(glm::vec3(
            randomFloats(randomEngine) * 2.0f - 1.0f,
            randomFloats(randomEngine) * 2.0f - 1.0f,
            0.0f
        ));
    }

    GLuint noiseTexture;
    glGenTextures(1, &noiseTexture);
    glActiveTexture(GL_TEXTURE0 + SSDO_NOISE_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 8, 8, 0, GL_RGB, GL_FLOAT, &noise[0]);

    return noiseTexture;
}

GLuint generateTexture() {
    return generateTexture(window_width, window_height);
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
    window_width = mode->width;
    window_height = mode->height;
    GLFWwindow *window;

    if (debug_flag) {
        window_width = DEFAULT_WIDTH;
        window_height = DEFAULT_HEIGHT;

        window = glfwCreateWindow(
            window_width, window_height, "Brechpunkt", nullptr, nullptr
        );
    } else {
        window = glfwCreateWindow(
            window_width, window_height, "Brechpunkt", monitor, nullptr
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
