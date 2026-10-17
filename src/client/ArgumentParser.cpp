#include "client/ArgumentParser.hpp"
#include <charconv>
#include <string_view>

namespace Client {

    static uint16_t parsePort(std::string_view text)
    {
        unsigned int port = 0;
        const char *end = text.data() + text.size();
        auto [ptr, error] = std::from_chars(text.data(), end, port);

        if (text.empty() || error != std::errc{} || ptr != end || port == 0 || port > 65535)
            throw ArgumentException("invalid port \"" + std::string(text) + "\": expected a number between 1 and 65535");
        return static_cast<uint16_t>(port);
    }

    ClientConfig ArgumentParser::parse(int argc, char **argv) const
    {
        ClientConfig config;

        for (int i = 1; i < argc; i++) {
            const std::string_view arg = argv[i];

            if (arg == "-h" || arg == "--help") {
                config.showHelp = true;
                return config;
            }
            if (arg != "-t" && arg != "--host" && arg != "-p" && arg != "--port")
                throw ArgumentException("unknown option \"" + std::string(arg) + "\"");
            if (i + 1 >= argc)
                throw ArgumentException("missing value after " + std::string(arg));

            const std::string_view value = argv[++i];
            if (arg == "-t" || arg == "--host") {
                if (value.empty())
                    throw ArgumentException("the host can't be empty");
                config.host = value;
            } else {
                config.port = parsePort(value);
            }
        }
        return config;
    }

    std::string ArgumentParser::usage(const std::string &programName)
    {
        const ClientConfig defaults;
        return "USAGE: " + programName + " [-t hostname] [-p port]\n"
            "\n"
            "OPTIONS:\n"
            "    -t, --host <hostname>   server to connect to (default: " + defaults.host + ")\n"
            "    -p, --port <port>       port of the server, 1 to 65535 (default: " + std::to_string(defaults.port) + ")\n"
            "    -h, --help              show this help and quit\n";
    }
}
