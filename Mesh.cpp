#include "Mesh.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RessourceManager.h"

Mesh::Mesh(GLuint VAO, std::string materialName, int numTriangles) :
	VAO(VAO),
	materialName(materialName),
	numTriangles(numTriangles)
{
}

void Mesh::draw(Shader *shader, glm::mat4 &modelMatrix) {
	shader->setTexture2D("tex", GL_TEXTURE0, RessourceManager::textures[RessourceManager::materials[materialName].diffuse_texname], 0);
	shader->setTexture2D("reflectionTex", GL_TEXTURE1, RessourceManager::textures[RessourceManager::materials[materialName].metallic_texname], 1);
	if (RessourceManager::materials[materialName].normal_texname.length() > 0) {
		shader->setInteger("useNormalTex", 1);
		shader->setTexture2D("normalTex", GL_TEXTURE2, RessourceManager::textures[RessourceManager::materials[materialName].normal_texname], 2);
	} else {
		shader->setInteger("useNormalTex", 0);
	}
	shader->setMatrix4("model", modelMatrix);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, numTriangles);
}

void Mesh::update(glm::mat4 &modelMatrix) {
	//modelMatrix = glm::translate(modelMatrix, glm::vec3(rigidBody->getCenterOfMassPosition().x(), rigidBody->getCenterOfMassPosition().y(), rigidBody->getCenterOfMassPosition().z()));
}
