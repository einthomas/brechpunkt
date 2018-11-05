#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "Mesh.h"

#include <stdexcept>

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include "RessourceManager.h"

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

    if (data != nullptr) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }

    return texture;
}

MeshInfo::MeshInfo(std::string basedir, std::string objFileName) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    tinyobj::LoadObj(&attrib, &shapes, &materials, &err, (basedir + objFileName).c_str(), basedir.c_str());
    if (!err.empty()) {
        throw std::runtime_error(err);
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

    std::vector<float> meshData;
    int indexOffset = 0;
    for (int k = 0; k < shapes[0].mesh.num_face_vertices.size(); k++) {
        unsigned int faceVertices = shapes[0].mesh.num_face_vertices[k];
        for (int m = 0; m < faceVertices; m++) {
            tinyobj::index_t index = shapes[0].mesh.indices[indexOffset + m];

            meshData.push_back(attrib.vertices[3 * index.vertex_index + 0]);
            meshData.push_back(attrib.vertices[3 * index.vertex_index + 1]);
            meshData.push_back(attrib.vertices[3 * index.vertex_index + 2]);

            meshData.push_back(attrib.normals[3 * index.normal_index + 0]);
            meshData.push_back(attrib.normals[3 * index.normal_index + 1]);
            meshData.push_back(attrib.normals[3 * index.normal_index + 2]);

            if (index.texcoord_index > -1) {
                meshData.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
                meshData.push_back(attrib.texcoords[2 * index.texcoord_index + 1]);
            } else {
                meshData.push_back(0);
                meshData.push_back(0);
            }
        }
        indexOffset += faceVertices;
    }

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

    numTriangles = meshData.size() / 8;
    materialName = materials[shapes[0].mesh.material_ids[0]].name;
}


Mesh::Mesh(MeshInfo meshInfo, glm::mat4 model, glm::vec3 diffuseColor, glm::vec3 emissionColor) :
    diffuseColor(diffuseColor),
    emissionColor(emissionColor),
    model(model),
    vao(meshInfo.VAO),
    count(meshInfo.numTriangles)
{
    auto material = RessourceManager::materials.at(meshInfo.materialName);
    useDiffuseTexture = !material.diffuse_texname.empty();
    useReflectionTexture = !material.metallic_texname.empty();
    useNormalTexture = !material.normal_texname.empty();

    if (useDiffuseTexture) {
        diffuseTexture =
            RessourceManager::textures.at(material.diffuse_texname);
    }
    if (useReflectionTexture) {
        reflectionTexture =
            RessourceManager::textures.at(material.metallic_texname);
    }
    if (useNormalTexture) {
        normalTexture =
            RessourceManager::textures.at(material.normal_texname);
    }
}

void Mesh::draw(Shader &shader) {
    if (useDiffuseTexture) {
        shader.setInteger("useDiffuseTex", 1);
        shader.setTexture2D(
            "diffuseTex", GL_TEXTURE0, diffuseTexture, 0
        );
    } else {
        shader.setInteger("useDiffuseTex", 0);
        shader.setVector3f(
            "diffuseColor", diffuseColor
        );
    }
    if (useReflectionTexture) {
        shader.setTexture2D(
            "reflectionTex", GL_TEXTURE1, reflectionTexture, 1
        );
    }
    if (useNormalTexture) {
		shader.setInteger("useNormalTex", 1);
        shader.setTexture2D(
            "normalTex", GL_TEXTURE2, normalTexture, 2
        );
	} else {
		shader.setInteger("useNormalTex", 0);
	}

    shader.setMatrix4("model", model);
	shader.setVector3f("diffuseColor", diffuseColor);
    shader.setVector3f("emissionColor", emissionColor);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, count);
}

Mesh::Mesh() {}
