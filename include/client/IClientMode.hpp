#ifndef ICLIENT_MODE_HPP_
#define ICLIENT_MODE_HPP_

#include "client/InputManager.hpp"

namespace Client {

    enum class ModeId {
        Title,
        Game,
        Menu
    };

    // What a mode asks the session to do with the stack of modes, after the frame.
    struct Transition {
        enum class Kind {
            None,
            Push,   // put target on top: the modes below keep running, covered
            Pop,    // remove the mode on top
            Switch, // replace the whole stack by target
            Quit
        };

        Kind kind = Kind::None;
        ModeId target = ModeId::Title;

        static Transition push(ModeId id) noexcept { return {Kind::Push, id}; }
        static Transition pop() noexcept { return {Kind::Pop, ModeId::Title}; }
        static Transition switchTo(ModeId id) noexcept { return {Kind::Switch, id}; }
        static Transition quit() noexcept { return {Kind::Quit, ModeId::Title}; }
    };

    // What the client is doing. The session keeps a stack of modes: the one on top gets the input,
    // the ones below keep running without it (the game stays connected under the menu), and they
    // all draw their UI, from the bottom up.
    class IClientMode {
        public:
            virtual ~IClientMode() = default;

            virtual void onEnter() {}
            virtual void onExit() {}
            // on top of the stack. dt: seconds since the last frame
            virtual void update(const InputManager &input, double dt) = 0;
            // under another mode: no input for it
            virtual void updateCovered(double dt) { (void)dt; }
            // ImGui calls, made between rlImGuiBegin and rlImGuiEnd by the renderer
            virtual void drawOverlay() = 0;
            // what it asked for since the last call, then forgets it
            virtual Transition takeTransition() noexcept = 0;
    };

    class AClientMode : public IClientMode {
        public:
            Transition takeTransition() noexcept override {
                Transition transition = _transition;
                _transition = {};
                return transition;
            }

        protected:
            void request(const Transition &transition) noexcept { _transition = transition; }

        private:
            Transition _transition;
    };
}

#endif
