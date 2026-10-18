#ifndef COMMON_ENTITY_TYPE_HPP_
#define COMMON_ENTITY_TYPE_HPP_

#include <cstdint>
#include <optional>
#include <string_view>

namespace Common {

    // Every kind of entity the server can send. Both sides name them the same way in their
    // configs (gameplay.cfg on the server, models.cfg on the client), so their ids always agree.
    enum class EntityType : uint16_t {
        Player,
        Drone,
        Saucer,
        Fighter,
        Count
    };

    std::string_view entityTypeName(EntityType type) noexcept;
    std::optional<EntityType> entityTypeFromName(std::string_view name) noexcept;
}

#endif
