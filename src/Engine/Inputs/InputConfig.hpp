#pragma once

#if defined(CLIENT_BUILD)
#include "InputManager/ClientInputManager.hpp"
using InputManager = ClientInputManager;

#elif defined(SERVER_BUILD)
#include "InputManager/ServerInputManager.hpp"
using InputManager = ServerInputManager;

#else
#error "You must compile with -DSERVER_BUILD or -DCLIENT_BUILD"
#endif