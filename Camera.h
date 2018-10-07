#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;

    Camera() = default;
    Camera(glm::vec3 pos);
    void update();
    glm::mat4 getViewMatrix();
    void processMouseMovement(double xPos, double yPos);

private:
    double lastMouseX;
    double lastMouseY;
    float yaw;
    float pitch;
    bool firstMouse = true;
};
