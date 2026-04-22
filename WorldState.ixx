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
		int countryId = -1;
	};

	export struct Country {
		int id;
		std::string name;
	};

	export class TileArray {
	private:
		std::vector<Tile> tiles;
		std::mdspan<Tile, std::dextents<size_t, 2>> span;
	public:
		TileArray(size_t width, size_t height) : tiles(width * height), span(tiles.data(), width, height) {
		}

		TileArray() {
		}

		Tile& operator()(size_t x, size_t y) { return span[std::array{ x,y }]; }
		const Tile& operator()(size_t x, size_t y) const { return span[std::array{ x,y }]; }

		size_t width() { return span.extent(0); }
		size_t height() { return span.extent(1); }
	};

	export class WorldState {
	private:
		TileArray _tiles;
		int mapSeed;

	public:
		WorldState(size_t width, size_t height) : _tiles(width, height) {
		}

		WorldState() {
		}

		TileArray& tiles() {
			return this->_tiles;
		}

		int getMapSeed() {
			return this->mapSeed;
		}

		void setMapSeed(int value) {
			this->mapSeed = value;
		}
	};

}