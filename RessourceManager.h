#pragma once

#include <unordered_map>
#include <string>
#include <GL/glew.h>
#include <tiny_obj_loader.h>

class RessourceManager {
public:
    static std::unordered_map<std::string, tinyobj::material_t> materials;
    static std::unordered_map<std::string, GLuint> textures;
};
