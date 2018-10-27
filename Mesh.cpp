#include "Mesh.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RessourceManager.h"

Mesh::Mesh(MeshInfo meshInfo, glm::vec3 position, glm::vec3 diffuseColor, glm::vec3 emissionColor) :
	meshInfo(meshInfo),
    position(position),
	diffuseColor(diffuseColor),
    emissionColor(emissionColor)
{
}

Mesh::Mesh(MeshInfo meshInfo, glm::mat4 model, glm::vec3 diffuseColor, glm::vec3 emissionColor) :
    meshInfo(meshInfo),
    model(model),
    diffuseColor(diffuseColor),
    emissionColor(emissionColor)
{
}

void Mesh::draw(Shader &shader) {
    auto& material = RessourceManager::materials[meshInfo.materialName];
    if (material.diffuse_texname.length() > 0) {
        shader.setInteger("useDiffuseTex", 1);
        shader.setTexture2D(
            "diffuseTex", GL_TEXTURE0,
            RessourceManager::textures[material.diffuse_texname], 0
        );
    } else {
        shader.setInteger("useDiffuseTex", 0);
        shader.setVector3f(
            "diffuseColor",
            material.diffuse[0], material.diffuse[1], material.diffuse[2]
        );
    }
    shader.setTexture2D(
        "reflectionTex", GL_TEXTURE1,
        RessourceManager::textures[material.metallic_texname], 1
    );
    if (material.normal_texname.length() > 0) {
		shader.setInteger("useNormalTex", 1);
        shader.setTexture2D(
            "normalTex", GL_TEXTURE2,
            RessourceManager::textures[material.normal_texname], 2
        );
	} else {
		shader.setInteger("useNormalTex", 0);
	}
    
    //glm::mat4 modelMatrix(1.0f);
    //modelMatrix = glm::translate(modelMatrix, position);
	//shader.setMatrix4("model", modelMatrix);
    shader.setMatrix4("model", model);
	shader.setVector3f("diffuseColor", diffuseColor);
    shader.setVector3f("emissionColor", emissionColor);
    //shader.setVector3f(
    //    "emissionColor",
    //    material.emission[0], material.emission[1], material.emission[2]
    //);

	glBindVertexArray(meshInfo.VAO);
	glDrawArrays(GL_TRIANGLES, 0, meshInfo.numTriangles);
}

Mesh::Mesh() {}
