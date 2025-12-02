/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Velocity
*/

#ifndef VELOCITY_HPP_
    #define VELOCITY_HPP_

namespace Ecs {

    struct Placement_t {
        double pos_x;
        double pos_y;
        float rotation;
    };

    struct Velocity_t {
        float x;
        float y;
    };

    class VelocitySystem {
        public:
            VelocitySystem() = default;
            ~VelocitySystem();

            void UpdatePosition(Placement_t& placement, const Velocity_t& velocity, float dt);
    };

}

#endif /* !VELOCITY_HPP_ */
