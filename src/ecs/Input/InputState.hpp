/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** InputState.hpp
*/

#pragma once

struct ActionState {
    bool pressed = false;
    bool justPressed = false;
    bool justReleased = false;
    float holdTime = 0.f;
    float lastReleaseHoldTime = 0.f;
};
