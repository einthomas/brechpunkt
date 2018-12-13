#ifndef SCENE_H
#define SCENE_H

#include <unordered_set>
#include <Mesh.h>

class Scene {
public:
    Scene();

    void draw(Program& program);

    std::unordered_set<Mesh*> objects;
};

#endif // SCENE_H
