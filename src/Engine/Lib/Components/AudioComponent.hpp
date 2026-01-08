#pragma once

#include <string>

struct AudioSourceComponent {
    static constexpr auto name = "AudioSourceComponent";
    std::string sound_name;
    bool loop = false;
    bool play_on_start = true;
    bool stop_requested = false;
    int assigned_sound_index = -1;

    std::string next_sound_name;
    bool next_sound_loop = false;

    bool destroy_entity_on_finish = false;
};
