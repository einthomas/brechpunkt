#ifndef SCENE_H
#define SCENE_H

#include <unordered_set>
#include "Mesh.h"

class Scene {
public:
    Scene();

    void draw(Program& program);
    void drawGlassObjects(Program& program);

    std::unordered_set<Mesh*> objects;
    std::unordered_set<Mesh*> glassObjects;
};

#endif // SCENE_H
