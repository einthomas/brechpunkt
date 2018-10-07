#pragma once

#include <map>
#include <string>
#include <GL\glew.h>
#include <tiny_obj_loader.h>

class RessourceManager {
public:
    static std::map<std::string, tinyobj::material_t> materials;
    static std::map<std::string, GLuint> textures;
};
