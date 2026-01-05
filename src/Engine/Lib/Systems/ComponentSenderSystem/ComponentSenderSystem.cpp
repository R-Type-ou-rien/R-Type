#include "ComponentSenderSystem.hpp"
#include "Context.hpp"
#include "ISystem.hpp"
#include "registry.hpp"

void ComponentSenderSystem::update(Registry& reg, system_context ctx) {
    ComponentPacket packet;
    SerializationContext s_ctx = {ctx.texture_manager};
    auto& component_pools = reg.getComponentPools();

    // if (!ctx.network_server.has_value() || !ctx.clients_id) {
    //     return;
    // }
    // Server& server = ctx.network_server.value();
    // auto& players = ctx.clients_id.value().get();

    for (auto& [type, pool] : component_pools) {
        auto updated_components = pool->getUpdatedEntities();
        if (updated_components.empty())
            continue;
        for (auto entity : updated_components) {
            packet = pool->createPacket(entity, s_ctx);
            packet.entity_guid = reg.getConstComponent<NetworkIdentity>(entity).guid;
            // for (auto& player : players) {
            //     if (!server.IsClientReady(player))
            //         continue;
            //     server.AddMessageToPlayer(GameEvents::S_SNAPSHOT, player, packet);
            // }
        }
    }
    return;
}