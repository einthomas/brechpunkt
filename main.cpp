#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "RessourceManager.h"
#include "Mesh.h"

using namespace std;

vector<Mesh> meshes;

GLuint loadTexture(std::string textureFileName);
void loadObj(std::string basedir, std::string objFileName);

int main() {
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwSwapInterval(1);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    window = glfwCreateWindow(
        //mode->width, mode->height,
		1280, 720, "Brechpunkt", nullptr, nullptr
    );
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        return -1;
    }

    loadObj("scenes/scene1/", "demolevel.obj");

    while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

	glfwTerminate();
    return 0;
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
            meshData.size() / 8.0f
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
