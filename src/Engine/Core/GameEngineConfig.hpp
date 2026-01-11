#pragma once

#if defined(SERVER_BUILD)
#include "ServerGameEngine.hpp"
using GameEngine = ServerGameEngine;

#elif defined(CLIENT_BUILD)
#include "ClientGameEngine.hpp"
using GameEngine = ClientGameEngine;

#else
#error "You must compile with -DSERVER_BUILD or -DCLIENT_BUILD"
#endif
