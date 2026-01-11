#include "InputManagerBase.hpp"

class ClientInputManager : public InputManagerBase<ClientInputManager> {
   public:
    void update(float dt);                                     // client only -> returns the packet ?
    void setWindowHasFocus(bool focus) { _hasFocus = focus; }  // client only

   private:
    bool isBindingActive(const InputBinding& binding) const;  // client only
    bool _hasFocus = true;                                    // client only
};
