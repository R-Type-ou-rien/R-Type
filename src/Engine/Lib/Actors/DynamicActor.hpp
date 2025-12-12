#include <string>
#include <utility>

#include "AActor.hpp"
#include "InputAction.hpp"

class DynamicActor : public AActor {
   public:
    DynamicActor(ECS& ecs, bool playable, const std::string name = "DynamicActor");

    void addResourceStat(const std::string res_name, ResourceStat& resource);

    float getMaxResourceStat(const std::string res_name);

    float getCurrentResourceStat(const std::string res_name);

    float getRegenResourceStat(const std::string res_name);

    void setMaxResourceStat(const std::string res_name, float max);

    void setCurrentResourceStat(const std::string res_name, float current);

    void setRegenResourceStat(const std::string res_name, float regen);

    bool hasResource(const std::string res_name);

    void addEmptyEffect(const std::string res_name, std::function<void()> effect);

    /** */

    void setVelocity(std::pair<float, float> velocity);

    std::pair<float, float> getvelocity();

    /** IA */
    // void setPattern(std::vector<std::pair<float, float>> way_points)
    // std::vector<std::pair<float, float>> getPattern()

    /** Controllable */
    void bindActionCallbackOnPressed(Action action_name, ActionCallback callback);

    void bindActionCallbackPressed(Action action_name, ActionCallback callback);

    void bindActionCallbackOnReleased(Action action_name, ActionCallback callback);

    void removeActionCallbackOnPressed(Action action_name);

    void removeActionCallbackPressed(Action action_name);

    void removeActionCallbackOnReleased(Action action_name);

    std::unordered_map<Action, ActionCallback> getActionCallbackOnPressed();

    std::unordered_map<Action, ActionCallback> getActionCallbackPressed();

    std::unordered_map<Action, ActionCallback> getActionCallbackOnReleased();
};