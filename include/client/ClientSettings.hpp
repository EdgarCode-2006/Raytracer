#ifndef CLIENT_SETTINGS_HPP_
#define CLIENT_SETTINGS_HPP_

namespace Client {

    // The settings of this client that its parts read every frame. The settings panel changes
    // them; the key bindings and the shader features have their own place.
    struct ClientSettings {
        // ships as flat bright silhouettes, a color per entity type, over a dimmed background
        bool highContrast = false;
    };

    // how bright the background is in high contrast mode
    inline constexpr double HIGH_CONTRAST_BACKGROUND = 0.3;
}

#endif
