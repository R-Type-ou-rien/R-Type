#include <cstdint>
#include "Components/NetworkComponents.hpp"
#include "InputAction.hpp"
#include "InputManagerBase.hpp"
#include "InputState.hpp"
#include "NetworkEngine/NetworkEngine.hpp"

class ClientInputManager : public InputManagerBase<ClientInputManager> {
   public:
    void update(engine::core::NetworkEngine& network, uint32_t tick, uint32_t dt);                                  // client only -> returns the packet ?
    void setWindowHasFocus(bool focus) { _hasFocus = focus; }  // client only

   private:
    void createActionPacket(Action name, ActionState state, engine::core::NetworkEngine& network);
    bool isBindingActive(const InputBinding& binding) const;  // client only
    bool _hasFocus = true;                                    // client only
};