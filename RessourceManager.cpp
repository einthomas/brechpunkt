#include "RessourceManager.h"

std::unordered_map<std::string, tinyobj::material_t>
    RessourceManager::materials;
std::unordered_map<std::string, GLuint>
    RessourceManager::textures;
