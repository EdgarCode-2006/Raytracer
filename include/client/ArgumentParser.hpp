#ifndef ARGUMENT_PARSER_HPP_
#define ARGUMENT_PARSER_HPP_

#include <cstdint>
#include <exception>
#include <string>

namespace Client {

    // Where the client connects to, set from the command line.
    struct ClientConfig {
        std::string host = "127.0.0.1";
        uint16_t port = 4242;
        bool showHelp = false; // -h: print the usage and quit
    };

    // Reads -t / --host <hostname> and -p / --port <portNumber> (each optional), and -h / --help.
    class ArgumentParser {
        public:
            ClientConfig parse(int argc, char **argv) const;
            static std::string usage(const std::string &programName);
    };

    class ArgumentException : public std::exception {
        public:
            ArgumentException(const std::string& message) : _message(message) {}
            const char* what() const noexcept override { return _message.c_str(); }
        private:
            std::string _message;
    };
}

#endif
