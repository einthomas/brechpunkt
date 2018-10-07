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
    if (RessourceManager::materials[materialName].diffuse_texname.length() > 0) {
        shader.setInteger("useDiffuseTex", 1);
        shader.setTexture2D("diffuseTex", GL_TEXTURE0, RessourceManager::textures[RessourceManager::materials[materialName].diffuse_texname], 0);
    } else {
        shader.setInteger("useDiffuseTex", 0);
    }
	shader.setTexture2D("reflectionTex", GL_TEXTURE1, RessourceManager::textures[RessourceManager::materials[materialName].metallic_texname], 1);
	if (RessourceManager::materials[materialName].normal_texname.length() > 0) {
		shader.setInteger("useNormalTex", 1);
		shader.setTexture2D("normalTex", GL_TEXTURE2, RessourceManager::textures[RessourceManager::materials[materialName].normal_texname], 2);
	} else {
		shader.setInteger("useNormalTex", 0);
	}

    glm::mat4 modelMatrix(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
	shader.setMatrix4("model", modelMatrix);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, numTriangles);
}
