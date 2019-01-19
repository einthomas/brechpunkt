#pragma once

#include <glm/glm.hpp>

#include "Animation.h"
#include "Placement.h"

static glm::quat FORWARD =
    glm::angleAxis(glm::radians(90.0f), glm::vec3{1, 0, 0});

static glm::quat SIDEWAYS =
    glm::angleAxis(glm::radians(90.0f), glm::vec3{0, 0, 1});

static float SWEEP_SPEED_FACTOR = 3.0f;

static Action<Placement> DOWN_SWEEP {
    {
        {0, {{0, 50, 0}, {1, 0, 1}}, HandleType::STOP},
        {1 * SWEEP_SPEED_FACTOR, {{0, 0, 0}, {1, 1, 1}}, HandleType::SMOOTH},
        {2 * SWEEP_SPEED_FACTOR, {{0, -50, 0}, {1, 0, 1}}, HandleType::STOP},
    }, 2, ActionType::STRETCH
};

static Action<Placement> FORWARD_SWEEP {
    {
        {0, {{0, 0, -100}, {1, 0, 1}, FORWARD}, HandleType::STOP},
        {1 * SWEEP_SPEED_FACTOR, {{0, 0, 0}, {1, 2, 1}, FORWARD}, HandleType::SMOOTH},
        {2 * SWEEP_SPEED_FACTOR, {{0, 0, 100}, {1, 0, 1}, FORWARD}, HandleType::STOP}
    }, 2, ActionType::STRETCH
};

static Action<Placement> SIDEWAYS_SWEEP {
    {
        {0, {{-100, 0, 0}, {1, 0, 1}, SIDEWAYS}, HandleType::STOP},
        {1 * SWEEP_SPEED_FACTOR, {{0, 0, 0}, {1, 2, 1}, SIDEWAYS}, HandleType::SMOOTH},
        {2 * SWEEP_SPEED_FACTOR, {{100, 0, 0}, {1, 0, 1}, SIDEWAYS}, HandleType::STOP}
    }, 2, ActionType::STRETCH
};

static Action<Placement> HORIZONTAL_ON {
    {
        {0, {{0, 10, 0}, {1, 0, 1}}, HandleType::STOP},
        {1, {{0, 10, 0}, {1, 1, 1}}, HandleType::STOP}
    }, 1, ActionType::STRETCH
};

static Action<Placement> HORIZONTAL_OFF {
    {
        {0, {{0, 20, 0}, {1, 1, 1}}, HandleType::STOP},
        {1, {{0, 20, 0}, {1, 0, 1}}, HandleType::STOP}
    }, 1, ActionType::STRETCH
};

static Action<glm::vec3> CLAP {
    {
        {0, {0.8, 0.8, 0.8}, HandleType::STOP},
        {0, {0.8, 0.8, 0.8}, HandleType::SMOOTH_OUT},
        {0.1f, {0.5, 0.5, 0.5}, HandleType::SMOOTH_IN},
        {0.1f, {0.5, 0.5, 0.5}, HandleType::STOP},
        {0.3f, {0.5, 0.5, 0.5}, HandleType::STOP},
        {1, {0.8, 0.8, 0.8}, HandleType::SMOOTH_IN},
        {1, {0.8, 0.8, 0.8}, HandleType::STOP},
    }, 1, ActionType::STRETCH
};

static Action<glm::vec3> RUBIKS_X {
    {
        {0, {0, 0, 0}, HandleType::STOP},
        {0, {0, 0, 0}, HandleType::SMOOTH_OUT},
        {1, {glm::radians(90.0f), 0, 0}, HandleType::STOP},
        {1, {0, 0, 0}, HandleType::STOP},
    }, 1, ActionType::STRETCH
};

static Action<glm::vec3> RUBIKS_Y {
    {
        {0, {0, 0, 0}, HandleType::STOP},
        {0, {0, 0, 0}, HandleType::SMOOTH_OUT},
        {1, {0, glm::radians(90.0f), 0}, HandleType::STOP},
        {1, {0, 0, 0}, HandleType::STOP},
    }, 1, ActionType::STRETCH
};

static Action<glm::vec3> RUBIKS_Z {
    {
        {0, {0, 0, 0}, HandleType::STOP},
        {0, {0, 0, 0}, HandleType::SMOOTH_OUT},
        {1, {0, 0, glm::radians(90.0f)}, HandleType::STOP},
        {1, {0, 0, 0}, HandleType::STOP},
    }, 1, ActionType::STRETCH
};
