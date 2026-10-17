#ifndef LOCAL_SERVER_LINK_HPP_
#define LOCAL_SERVER_LINK_HPP_

#include "client/IServerLink.hpp"
#include "simulation/GameSimulation.hpp"
#include <atomic>
#include <mutex>
#include <thread>

namespace Client {

    // A server running inside the client, on its own thread at the server's tick rate. It behaves
    // like the network one will: inputs and states only cross through the locked buffers below,
    // and a state can arrive or not on any frame.
    class LocalServerLink : public IServerLink {
        public:
            explicit LocalServerLink(Simulation::GameRules rules);
            ~LocalServerLink() override;

            LocalServerLink(const LocalServerLink &) = delete;
            LocalServerLink &operator=(const LocalServerLink &) = delete;

            void sendInput(const Common::InputState &input) override;
            std::optional<Common::WorldState> takeLatestState() override;
            bool isConnected() const noexcept override { return _running; }
            std::string describe() const override { return "local server"; }

        private:
            void serverLoop();

            Simulation::GameSimulation _simulation; // only used by the server thread after the constructor
            uint32_t _playerId;

            std::mutex _mutex;
            Common::InputState _pendingInput;
            std::optional<Common::WorldState> _latestState;

            std::atomic<bool> _running{true};
            std::thread _thread; // last: starts once everything above is ready
    };
}

#endif
