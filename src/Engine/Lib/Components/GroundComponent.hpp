/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** GroundComponent.hpp
*/

#pragma once

#include "StructDatas/Rect2D.hpp"

struct GroundComponent
{
    static constexpr auto name = "GroundComponent";
    Rect2D rect;
    bool isSolid;
};