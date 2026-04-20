export module WorldState;

import std;

export namespace OpenKaiser {
	export enum class TileType {
		Grass,
		Mountain,
		Water
	};

	export struct Tile {
		TileType type;
	};

	export class WorldState {
	private:
		std::vector<Tile> _tiles;
		size_t _width;
		size_t _height;

	public:
		WorldState(size_t width, size_t height) : _tiles(width * height) {
			// Initialize the world with some default tiles (e.g., all grass)
			std::fill(_tiles.begin(), _tiles.end(), Tile{TileType::Grass});
			_width = width;
			_height = height;
		}

		WorldState() {
			_width = 0;
			_height = 0;

		}

		auto tiles() {
			return std::mdspan(_tiles.data(), _width, _height);
		}
	};

}