#include "Scene.h"

Scene::Scene() {

}

void Scene::draw(Program &program) {
    for (auto o : objects) {
        o->draw(program);
    }
}
