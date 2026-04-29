export module General;

import std;

namespace OpenKaiser {

    export class OpenKaiserError : public std::runtime_error {
    public:
        OpenKaiserError(const std::string& msg)
            : std::runtime_error(msg) {}
    };
}