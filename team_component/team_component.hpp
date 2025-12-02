#pragma one

#ifndef TEAM_COMPONENT_TEAM_COMPONENT_HPP_
#define TEAM_COMPONENT_TEAM_COMPONENT_HPP_

namespace ECS {
struct TeamComponent {
    enum Team { ALLY, ENEMY };
    Team team;
};
}  // namespace ECS

#endif  // TEAM_COMPONENT_TEAM_COMPONENT_HPP_
