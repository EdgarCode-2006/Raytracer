#ifndef ICLIENT_MODE_HPP_
#define ICLIENT_MODE_HPP_

#include "client/InputManager.hpp"

namespace Client {

    enum class ModeId {
        Title,
        Game,
        Quit
    };

    // What the client is doing: it gets the input, decides what the scene shows and draws its UI.
    class IClientMode {
        public:
            virtual ~IClientMode() = default;

            virtual void onEnter() {}
            virtual void onExit() {}
            // dt: seconds since the last frame
            virtual void update(const InputManager &input, double dt) = 0;
            // ImGui calls, made between rlImGuiBegin and rlImGuiEnd by the renderer
            virtual void drawOverlay() = 0;
            // the mode to run on the next frame, itself to keep running
            virtual ModeId next() const noexcept = 0;
    };
}

#endif
