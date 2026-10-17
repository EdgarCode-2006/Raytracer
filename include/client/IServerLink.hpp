#ifndef ISERVER_LINK_HPP_
#define ISERVER_LINK_HPP_

#include "common/InputState.hpp"
#include "common/WorldState.hpp"
#include <optional>
#include <string>

namespace Client {

    // The client's only way to talk to the game: it sends what the player presses and gets
    // back the state of the world. Whether the server runs in this process (LocalServerLink)
    // or across the network is hidden behind it.
    class IServerLink {
        public:
            virtual ~IServerLink() = default;

            virtual void sendInput(const Common::InputState &input) = 0;
            // the newest world state received since the last call, nothing if none arrived
            virtual std::optional<Common::WorldState> takeLatestState() = 0;

            virtual bool isConnected() const noexcept = 0;
            virtual std::string describe() const = 0; // shown to the player, "local server" for example
    };
}

#endif
