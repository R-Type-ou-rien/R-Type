#include "Components/NetworkComponents.hpp"
#include "InputManagerBase.hpp"
#include <unordered_map>

class ServerInputManager : public InputManagerBase<ServerInputManager> {
   public:
    int updateActionFromPacket(ActionPacket packet, uint32_t client_id);

    bool isPressed(Action action, uint32_t client_id) const;
    bool isJustPressed(Action action, uint32_t client_id) const;
    bool isJustReleased(Action action, uint32_t client_id) const;
    const ActionState& getState(Action action, uint32_t client_id) const;

    void removeClient(uint32_t client_id);
    void resetFrameFlags();  // Must be called at end of each frame to reset justPressed/justReleased

   private:
    const ActionState& getClientState(Action action, uint32_t client_id) const;

    std::unordered_map<uint32_t, std::unordered_map<Action, ActionState>> _client_states;
    ActionState _default_state;
};