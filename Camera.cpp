#include "Camera.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Camera::Camera(glm::vec3 pos) :
    pos(pos) {
    yaw = 90.0f;
    pitch = 90.0f;
}

void Camera::update() {
    front = glm::vec3(		// cartesian to spherical coordinates
        sin(glm::radians(pitch)) * cos(glm::radians(yaw)),
        cos(glm::radians(pitch)),
        sin(glm::radians(pitch)) * sin(glm::radians(yaw))
    );
    front = glm::normalize(front);
    right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    up = glm::normalize(glm::cross(front, right));
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(pos, pos + front, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::processMouseMovement(double posX, double posY) {
    if (firstMouse) {
        lastMouseX = posX;
        lastMouseY = posY;
        firstMouse = false;
    }

    double mouseDeltaX = posX - lastMouseX;
    double mouseDeltaY = posY - lastMouseY;
    lastMouseX = posX;
    lastMouseY = posY;

    yaw += mouseDeltaX * 0.1f;
    pitch += mouseDeltaY * 0.1f;

    pitch = std::min(std::max(pitch, 0.01f), 179.99f);
}
