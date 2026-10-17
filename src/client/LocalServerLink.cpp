#include "client/LocalServerLink.hpp"
#include "common/World.hpp"
#include <chrono>

namespace Client {

    LocalServerLink::LocalServerLink(Simulation::GameRules rules)
        : _simulation(std::move(rules)), _playerId(_simulation.addPlayer()),
        _thread(&LocalServerLink::serverLoop, this)
    {
    }

    LocalServerLink::~LocalServerLink()
    {
        _running = false;
        if (_thread.joinable())
            _thread.join();
    }

    void LocalServerLink::sendInput(const Common::InputState &input)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _pendingInput = input;
    }

    std::optional<Common::WorldState> LocalServerLink::takeLatestState()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::optional<Common::WorldState> state = std::move(_latestState);
        _latestState.reset();
        return state;
    }

    void LocalServerLink::serverLoop()
    {
        using Clock = std::chrono::steady_clock;
        const auto tickDuration = std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(1.0 / Common::TICK_RATE));
        auto nextTick = Clock::now();

        while (_running) {
            {
                std::lock_guard<std::mutex> lock(_mutex);
                _simulation.setInput(_playerId, _pendingInput);
            }
            _simulation.step(1.0f / Common::TICK_RATE);
            Common::WorldState state = _simulation.state(_playerId);
            {
                std::lock_guard<std::mutex> lock(_mutex);
                _latestState = std::move(state); // a state the client didn't take yet is replaced
            }

            nextTick += tickDuration;
            const auto now = Clock::now();
            if (nextTick < now)
                nextTick = now; // late (debugger, stall): don't run many ticks to catch up
            std::this_thread::sleep_until(nextTick);
        }
    }
}
