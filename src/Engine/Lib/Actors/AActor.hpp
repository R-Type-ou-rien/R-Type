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

     
    std::vector<std::string> getTags();

    void setTags(const std::vector<std::string> tags);

    void addTag(const std::string tag);

    void removeTag(const std::string tag);

     
    std::pair<float, float> getPosition();

    void setPosition(std::pair<float, float> pos);

    float getRotation();

    void setRotation(float rotation);

    void setScale(std::pair<float, float> scale);

    std::pair<float, float> getScale();

     

    void setTextureEnemy(const std::string pathname);

    void setTextureBoss(const std::string pathname);

    void setTexture(const std::string pathname);

    void setTextureDimension(rect dimension);

    rect getDimension();

    void setAnimation(bool state);

    bool isAnimmated();

    void setAnimationSpeed(float speed);

    float getAnimationSpeed();

    void setDisplayLayer(int layer);

    int getDisplayLayer();

     
    void setCollisionTags(std::vector<std::string> tags);

    void addCollisionTag(const std::string tag);

    void removeCollisionTag(const std::string tag);

    void emptyCollisionTags();
};