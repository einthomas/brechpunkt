#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "RessourceManager.h"
#include "Mesh.h"
#include "Animation.h"
#include "Camera.h"

using namespace std;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const float CAMERA_SPEED = 10.0f;

Camera camera;
Shader gBufferShader;
Shader composeShader;
vector<Mesh> meshes;
float deltaTime;
bool useAnimatedCamera = false;

GLuint loadTexture(std::string textureFileName);
void loadObj(std::string basedir, std::string objFileName);
GLFWwindow *initGLFW();
bool initGLEW();
GLuint getScreenQuadVAO();
GLuint generateTexture();
GLuint generateTexture(int width, int height);
void drawScreenQuad(GLuint screenQuadVAO);
void mouseCallback(GLFWwindow* window, double xPos, double yPos);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

int main() {
    GLFWwindow* window = initGLFW();
    if (window == nullptr) {
        return -1;
    }
    if (!initGLEW()) {
		return -1;
	}
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);

    GLuint gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    GLuint gColor = generateTexture();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gColor, 0);
    GLuint gWorldPos = generateTexture();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gWorldPos, 0);
    GLuint gNormal = generateTexture();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gNormal, 0);
    GLuint gReflection = generateTexture();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gReflection, 0);
    GLuint gBufferAttachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, gBufferAttachments);
    GLuint depthRBO;
    glGenRenderbuffers(1, &depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint screenQuadVAO = getScreenQuadVAO();

    loadObj("scenes/scene1/", "demolevel.obj");

    float near = 0.1f;
    float far = 100.0f;
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, near, far);

    Animation<glm::vec3> cameraPosition {
        {0, {0.8, 0.5, 4}},
        {4, {3, 0.5, 4}},
        {8, {0, 0.5, 4}},
    };

    gBufferShader = Shader("shaders/gBuffer.vert", "shaders/gBuffer.frag");
    composeShader = Shader("shaders/compose.vert", "shaders/compose.frag");

    camera = Camera(glm::vec3(0.0f, 1.0f, 0.0f));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

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
        
        glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gBufferShader.use();
        gBufferShader.setMatrix4("viewProjectionMatrix", viewProjectionMatrix);

        for (int i = 0; i < meshes.size(); i++) {
            meshes[i].draw(gBufferShader);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        composeShader.use();
        composeShader.setTexture2D("colorTex", GL_TEXTURE0, gColor, 0);
        drawScreenQuad(screenQuadVAO);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

	glfwTerminate();
    return 0;
}

void mouseCallback(GLFWwindow* window, double xPos, double yPos) {
    camera.processMouseMovement(xPos, yPos);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        gBufferShader = Shader("shaders/gBuffer.vert", "shaders/gBuffer.frag");
        composeShader = Shader("shaders/compose.vert", "shaders/compose.frag");
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        useAnimatedCamera = !useAnimatedCamera;
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

GLuint generateTexture() {
    return generateTexture(WINDOW_WIDTH, WINDOW_HEIGHT);
}

GLuint generateTexture(int width, int height) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    return texture;
}

GLFWwindow *initGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    //GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    //const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    GLFWwindow *window = glfwCreateWindow(
        //mode->width, mode->height
        WINDOW_WIDTH, WINDOW_HEIGHT, "Brechpunkt", nullptr, nullptr
    );
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
