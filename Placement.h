#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

struct Placement {
    Placement();
    Placement(
        glm::vec3 position,
        glm::vec3 scale = {1, 1, 1},
        glm::quat orientation = {1, 0, 0, 0}
    );

    glm::vec3 position;
    glm::vec3 scale;
    glm::quat orientation;

    Placement operator+(Placement o) const;
    Placement operator*(float s) const;
    glm::mat4 to_matrix() const;
};


Placement::Placement() : position(0), scale(0), orientation(0, 0, 0, 0) {}

Placement::Placement(
    glm::vec3 position, glm::vec3 scale, glm::quat orientation
) :
    position(position), scale(scale), orientation(orientation)
{}

inline Placement Placement::operator+(Placement o) const {
    return {
        position + o.position, scale + o.scale, orientation + o.orientation
    };
}

inline Placement Placement::operator*(float s) const {
    return {
        position * s, scale * s, orientation * s
    };
}

inline glm::mat4 Placement::to_matrix() const {
    glm::mat4 m = glm::scale(glm::mat4(1.0), scale);
    m = glm::toMat4(glm::normalize(orientation)) * m;
    m = glm::translate(glm::mat4(1.0), position) * m;
    return m;
}
