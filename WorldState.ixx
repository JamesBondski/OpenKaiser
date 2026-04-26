export module WorldState;

import std;
import SDL3;

export namespace OpenKaiser {
	export enum class TileType : std::uint8_t {
		Grass,
		Mountain,
		Water
	};

	export enum class BuildingType : std::uint8_t {
		None,
		Village,
		Castle,
		Town,
		Palace,
		Field,
		Pasture
	};

	export struct Tile {
		TileType type=TileType::Grass;
		std::int16_t countryId = -1;
		BuildingType building = BuildingType::None;
		std::uint32_t population=0;
	};

	export struct Country {
		std::int16_t id;
		std::string name;
		sdl::Point capital;
	};

	export template<typename T> class Array2D {
	private:
		std::vector<T> tiles;
		size_t width_;
		size_t height_;
	public:
		Array2D(size_t width, size_t height) : tiles(width * height), width_(width), height_(height) {
		}

		Array2D() : width_(0), height_(0) {
		}

		T& operator()(size_t x, size_t y) { return tiles[x * height_ + y]; }
		const T& operator()(size_t x, size_t y) const { return tiles[x * height_ + y]; }

		size_t width() { return width_; }
		size_t height() { return height_; }

		std::vector<T>& getTiles() {
			return this->tiles;
		}
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

		void save(const std::string& path) {
			std::ofstream save_file(path);
			save_file << this->mapSeed << std::endl;
			save_file << this->countries_.size() << std::endl;
			for (Country& country : this->countries_) {
				save_file << country.id << " " << country.name << " " << country.capital.x << " " << country.capital.y << std::endl;
			}
			save_file << this->_tiles.width() << " " << this->_tiles.height() << std::endl;
			for (Tile tile : this->_tiles.getTiles()) {
				save_file << static_cast<int>(tile.type) << " " << static_cast<int>(tile.building) << " " << tile.countryId << " " << tile.population << std::endl;
			}
		}

		void load(const std::string& path) {
			std::ifstream save_file(path);
			save_file >> this->mapSeed;
			int numCountries;
			save_file >> numCountries;

			for (int i = 0; i < numCountries; i++) {
				Country country;
				int x, y;
				save_file >> country.id >> country.name >> x >> y;
				country.capital.x = x;
				country.capital.y = y;
				this->countries().push_back(country);
			}

			int width, height;
			save_file >> width >> height;
			this->_tiles = Array2D<Tile>(width, height);

			for (int i = 0; i < width * height; i++) {
				Tile tile;
				int tileType, buildingType;
				save_file >> tileType >> buildingType >> tile.countryId >> tile.population;
				tile.type = static_cast<TileType>(tileType);
				tile.building = static_cast<BuildingType>(buildingType);
				this->_tiles.getTiles()[i] = tile;
			}
		}
	};

}