#pragma once

#include <glm/glm.hpp>

#include "Animation.h"
#include "Placement.h"

static glm::quat FORWARD =
    glm::angleAxis(glm::radians(90.0f), glm::vec3{1, 0, 0});

static glm::quat SIDEWAYS =
    glm::angleAxis(glm::radians(90.0f), glm::vec3{0, 0, 1});

static Action<Placement> DOWN_SWEEP {
    {
        {0, {{0, 50, 0}, {1, 0, 1}}, HandleType::STOP},
        {0.5, {{0, 0, 0}, {1, 1, 1}}, HandleType::SMOOTH},
        {1, {{0, -50, 0}, {1, 0, 1}}, HandleType::STOP},
    }, 1, ActionType::STRETCH
};

static Action<Placement> FORWARD_SWEEP {
    {
        {0, {{0, 0, -100}, {1, 0, 1}, FORWARD}, HandleType::STOP},
        {0.5, {{0, 0, 0}, {1, 2, 1}, FORWARD}, HandleType::SMOOTH},
        {1, {{0, 0, 100}, {1, 0, 1}, FORWARD}, HandleType::STOP}
    }, 1, ActionType::STRETCH
};

static Action<Placement> SIDEWAYS_SWEEP {
    {
        {0, {{-100, 0, 0}, {1, 0, 1}, SIDEWAYS}, HandleType::STOP},
        {0.5, {{0, 0, 0}, {1, 2, 1}, SIDEWAYS}, HandleType::SMOOTH},
        {1, {{100, 0, 0}, {1, 0, 1}, SIDEWAYS}, HandleType::STOP}
    }, 1, ActionType::STRETCH
};
