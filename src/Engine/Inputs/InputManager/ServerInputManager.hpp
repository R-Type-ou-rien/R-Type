#include "Components/NetworkComponents.hpp"
#include "InputManagerBase.hpp"

class ServerInputManager : public InputManagerBase<ServerInputManager> {
    public:
        void update(float dt) {}
        int updateActionFromPacket(ActionPacket packet);
};