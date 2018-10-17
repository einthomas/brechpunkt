#include "Mesh.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RessourceManager.h"

Mesh::Mesh(GLuint VAO, std::string materialName, int numTriangles, glm::vec3 position) :
	VAO(VAO),
	materialName(materialName),
	numTriangles(numTriangles),
    position(position)
{
}

void Mesh::draw(Shader &shader) {
    auto& material = RessourceManager::materials[materialName];
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
    shader.setVector3f(
        "emissionColor",
        material.emission[0], material.emission[1], material.emission[2]
    );

    glm::mat4 modelMatrix(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
	shader.setMatrix4("model", modelMatrix);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, numTriangles);
}
