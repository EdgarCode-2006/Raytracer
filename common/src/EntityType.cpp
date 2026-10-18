#include "common/EntityType.hpp"
#include <array>
#include <cstddef>

namespace Common {

    static constexpr std::array<std::string_view, static_cast<std::size_t>(EntityType::Count)> NAMES = {
        "player",
        "drone",
        "saucer",
        "fighter",
        "boss",
    };

    std::string_view entityTypeName(EntityType type) noexcept
    {
        const auto index = static_cast<std::size_t>(type);
        return index < NAMES.size() ? NAMES[index] : "unknown";
    }

    std::optional<EntityType> entityTypeFromName(std::string_view name) noexcept
    {
        for (std::size_t i = 0; i < NAMES.size(); i++)
            if (NAMES[i] == name)
                return static_cast<EntityType>(i);
        return std::nullopt;
    }
}
