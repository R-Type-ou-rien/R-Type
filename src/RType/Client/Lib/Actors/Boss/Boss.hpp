/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Boss
*/

#ifndef BOSS_HPP_
#define BOSS_HPP_

#include <string>
#include <utility>
#include "DynamicActor.hpp"
#include "Components/StandardComponents.hpp"
#include "../../../../Common/Components/team_component.hpp"
#include "../../../../Common/Components/shooter.hpp"

class Boss : public DynamicActor {
   public:
    explicit Boss(ECS& ecs);
    ~Boss();

   protected:
   private:
};

#endif /* !BOSS_HPP_ */
