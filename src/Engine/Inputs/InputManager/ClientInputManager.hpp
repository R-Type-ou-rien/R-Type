#pragma once

#include <cstdint>
#include "Components/NetworkComponents.hpp"
#include "InputAction.hpp"
#include "InputManagerBase.hpp"
#include "InputState.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
#include "Components/PredictionComponent.hpp"

struct system_context;

class ClientInputManager : public InputManagerBase<ClientInputManager> {
   public:
    void update(engine::core::NetworkEngine& network, uint32_t tick,
                system_context& ctx);  // client only -> returns the packet ?
    void setWindowHasFocus(bool focus) { _hasFocus = focus; }
    InputSnapshot getCurrentInputSnapshot() const;

   private:
    void createActionPacket(Action name, ActionState state, engine::core::NetworkEngine& network, system_context& ctx);
    bool isBindingActive(const InputBinding& binding) const;  // client only
    bool _hasFocus = true;                                    // client only
};
