export module WorldState;

import std;

export namespace OpenKaiser {
	export enum class TileType : std::uint8_t {
		Grass,
		Mountain,
		Water
	};

	export enum class BuildingType : std::uint8_t {
		Village,
		Castle,
		Town,
		Palace
	};

	export struct Tile {
		TileType type;
		std::uint8_t countryId = -1;
		BuildingType building;
		std::uint32_t population;
	};

	export struct Country {
		std::uint8_t id;
		std::string name;
	};

	export template<typename T> class Array2D {
	private:
		std::vector<T> tiles;
		std::mdspan<T, std::dextents<size_t, 2>> span;
	public:
		Array2D(size_t width, size_t height) : tiles(width * height), span(tiles.data(), width, height) {
		}

		Array2D() {
		}

		T& operator()(size_t x, size_t y) { return span[std::array{ x,y }]; }
		const T& operator()(size_t x, size_t y) const { return span[std::array{ x,y }]; }

		size_t width() { return span.extent(0); }
		size_t height() { return span.extent(1); }
	};

	export class WorldState {
	private:
		Array2D<Tile> _tiles;
		int mapSeed;
		std::vector<Country> countries_;

	public:
		WorldState(size_t width, size_t height) : _tiles(width, height) {
		}

		WorldState() {
		}

		Array2D<Tile>& tiles() {
			return this->_tiles;
		}

		std::vector<Country>& countries() {
			return this->countries_;
		}

		int getMapSeed() {
			return this->mapSeed;
		}

		void setMapSeed(int value) {
			this->mapSeed = value;
		}
	};

}