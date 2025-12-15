#include <string>
#include <utility>
#include "DynamicActor.hpp"

class ProjectileActor : public DynamicActor {
   public:
    ProjectileActor(ECS& ecs, std::pair<float, float> pos, const std::string name);
};
