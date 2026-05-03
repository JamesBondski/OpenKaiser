export module General;

import std;

namespace OpenKaiser {

    export class OpenKaiserError : public std::runtime_error {
    public:
        OpenKaiserError(const std::string& msg)
            : std::runtime_error(msg) {}
    };

    export struct Coordinates {
        int x;
        int y;

        bool operator==(const Coordinates&) const = default;
        auto operator<=>(const Coordinates&) const = default;
    };
    
    export enum class Adjacency {
        Left = 0,
        Right = 1,
        Top = 2,
        Bottom = 3
    };

    export inline std::array<Coordinates, 4> GetAdjacentTiles(const Coordinates& coord) {
        return { {
            {coord.x - 1, coord.y},
            {coord.x + 1, coord.y},
            {coord.x, coord.y - 1},
            {coord.x, coord.y + 1}
        } };
    }
}

namespace std {
    template<>
    struct hash<OpenKaiser::Coordinates> {
        size_t operator()(const OpenKaiser::Coordinates& coord) const noexcept {
            size_t h1 = std::hash<int>{}(coord.x);
            size_t h2 = std::hash<int>{}(coord.y);
            return h1 ^ (h2 << 1);
        }
    };
}