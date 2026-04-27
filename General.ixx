export module General;

import std;
import SDL3;
import ResourceManager;
import WorldState;
import GameController;

namespace OpenKaiser {

    export class OpenKaiserError : public std::runtime_error {
    public:
        OpenKaiserError(const std::string& msg)
            : std::runtime_error(msg) {}
    };
}