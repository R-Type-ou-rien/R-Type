#include <string>
#include <utility>
#include <vector>

#include "Components/StandardComponents.hpp"
#include "ECS/ECS.hpp"
#include "registry.hpp"

class AActor {
   protected:
    ECS& _ecs;
    Entity _id;

   public:
    AActor(ECS& ecs, const std::string name);

    virtual ~AActor();

    Entity getId();

    /** Tag component's methods */
    std::vector<std::string> getTags();

    void setTags(const std::vector<std::string> tags);

    void addTag(const std::string tag);

    void removeTag(const std::string tag);

    /** Transform component's methods */
    std::pair<float, float> getPosition();

    void setPosition(std::pair<float, float> pos);

    float getRotation();

    void setRotation(float rotation);

    void setScale(std::pair<float, float> scale);

    std::pair<float, float> getScale();

    /** Sprite2D component's methods */
    void setTexture(const std::string pathname);

    void setTextureDimension(rect dimension);

    rect getDimension();

    void setAnimation(bool state);

    bool isAnimmated();

    void setAnimationSpeed(float speed);

    float getAnimationSpeed();

    void setDisplayLayer(int layer);

    int getDisplayLayer();

    /** Collision component's methods */
    void setCollisionTags(std::vector<std::string> tags);

    void addCollisionTag(const std::string tag);

    void removeCollisionTag(const std::string tag);

    void emptyCollisionTags();
};