#include "Scene.h"

Scene::Scene() = default;

void Scene::draw(Program &program) {
    for (auto o : objects) {
        o->draw(program);
    }
}

void Scene::drawGlassObjects(Program & program) {
    for (auto o : glassObjects) {
        o->draw(program);
    }
}
