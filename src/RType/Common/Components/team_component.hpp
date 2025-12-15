#pragma once

#ifndef TEAM_COMPONENT_TEAM_COMPONENT_HPP_
#define TEAM_COMPONENT_TEAM_COMPONENT_HPP_

struct TeamComponent {
    static constexpr auto name = "TeamComponent";
    enum Team { ALLY, ENEMY };
    Team team;
};

#endif  // TEAM_COMPONENT_TEAM_COMPONENT_HPP_
