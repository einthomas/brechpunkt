#include "Mesh.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RessourceManager.h"

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
        shader.setInteger("useReflectionTex", 1);
        shader.setTexture2D(
            "reflectionTex", GL_TEXTURE1, reflectionTexture, 1
        );
    } else {
        shader.setInteger("useReflectionTex", 0);
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
