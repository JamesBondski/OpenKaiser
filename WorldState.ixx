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
		Array2D<Tile> tiles_;
		int map_seed_;
		std::vector<Country> countries_;
		std::int16_t current_country_id_ = 0;
		int year_ = 1000;

	public:
		WorldState(size_t width, size_t height) : tiles_(width, height) {
		}

		WorldState() {
		}

		std::int16_t current_country_id() const {
			return current_country_id_;
		}

		Array2D<Tile>& tiles() {
			return tiles_;
		}

		std::vector<Country>& countries() {
			return countries_;
		}

		int map_seed() const {
			return map_seed_;
		}

		void set_map_seed(int value) {
			map_seed_ = value;
		}

		int year() const {
			return year_;
		}

		int next_year() {
			return ++year_;
		}

		std::int16_t next_player() {
			current_country_id_++;
			if (current_country_id_ >= countries_.size()) {
				current_country_id_ = 0;
			}
			return current_country_id_;
		}

		void Save(const std::string& path) {
			std::ofstream save_file(path);
			save_file << map_seed_ << std::endl;
			save_file << year_ << std::endl;
			save_file << countries_.size() << std::endl;
			for (Country& country : countries_) {
				save_file << country.id << " " << country.name << " " << country.capital.x << " " << country.capital.y << std::endl;
			}
			save_file << current_country_id_ << std::endl;
			save_file << tiles_.width() << " " << tiles_.height() << std::endl;
			for (Tile tile : tiles_.getTiles()) {
				save_file << static_cast<int>(tile.type) << " " << static_cast<int>(tile.building) << " " << tile.countryId << " " << tile.population << std::endl;
			}
		}

		void Load(const std::string& path) {
			std::ifstream save_file(path);
			save_file >> map_seed_;
			save_file >> year_;

			int numCountries;
			save_file >> numCountries;
			for (int i = 0; i < numCountries; i++) {
				Country country;
				int x, y;
				save_file >> country.id >> country.name >> x >> y;
				country.capital.x = x;
				country.capital.y = y;
				countries().push_back(country);
			}

			save_file >> current_country_id_;

			int width, height;
			save_file >> width >> height;
			tiles_ = Array2D<Tile>(width, height);

			for (int i = 0; i < width * height; i++) {
				Tile tile;
				int tileType, buildingType;
				save_file >> tileType >> buildingType >> tile.countryId >> tile.population;
				tile.type = static_cast<TileType>(tileType);
				tile.building = static_cast<BuildingType>(buildingType);
				tiles_.getTiles()[i] = tile;
			}
		}
	};

}