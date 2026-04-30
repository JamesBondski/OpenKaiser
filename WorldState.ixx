export module WorldState;

import std;
import SDL3;
import General;

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
		Pasture,
		Market,
		Harbor
	};

	export enum class ResourceType : std::uint8_t {
		Gold,
		Wheat,
		Livestock
	};

	export struct Tile {
		TileType type=TileType::Grass;
		std::int16_t countryId = -1;
		BuildingType building = BuildingType::None;
		int population=0;
	};

	export struct Country {
		std::int16_t id;
		std::int16_t dynasty_id;
		std::string name;
		sdl::Point capital;
		std::unordered_map<ResourceType, int> resources;

		// Non-Persistant fields
		int population = 0;
		int population_fed = 0;
	};

	export struct Dynasty {
		std::int16_t id;
		bool human;
		std::string name;
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
		std::vector<Dynasty> dynasties_;
		std::int16_t current_country_id_ = 0;
		int year_ = 1000;

		const int save_file_version = 1;
	public:
		WorldState(size_t width, size_t height) : tiles_(width, height) {
		}

		WorldState() {
		}

		std::int16_t current_country_id() const {
			return current_country_id_;
		}

		Country& current_country() {
			return countries_[current_country_id_];
		}

		Dynasty& current_dynasty() {
			return dynasties_[countries_[current_country_id_].dynasty_id];
		}

		Array2D<Tile>& tiles() {
			return tiles_;
		}

		Tile& tile(size_t x, size_t y) {
			return tiles_(x, y);
		}

		std::vector<Country>& countries() {
			return countries_;
		}

		Country& country(std::int16_t id) {
			return countries_[id];
		}

		std::vector<Dynasty>& dynasties() {
			return dynasties_;
		}

		Dynasty& dynasty(std::int16_t id) {
			return dynasties_[id];
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
			save_file << save_file_version << std::endl;
			save_file << map_seed_ << std::endl;
			save_file << year_ << std::endl;
			save_file << dynasties_.size() << std::endl;
			for (Dynasty& dynasty : dynasties_) {
				save_file << dynasty.id << " " << dynasty.name << " " << dynasty.human << std::endl;
			}
			save_file << countries_.size() << std::endl;
			for (Country& country : countries_) {
				save_file << country.id << " " << country.name << " " << country.dynasty_id << " " << country.capital.x << " " << country.capital.y << std::endl;
			}
			save_file << current_country_id_ << std::endl;
			save_file << tiles_.width() << " " << tiles_.height() << std::endl;
			for (Tile tile : tiles_.getTiles()) {
				save_file << static_cast<int>(tile.type) << " " << static_cast<int>(tile.building) << " " << tile.countryId << " " << tile.population << std::endl;
			}
		}

		void Load(const std::string& path) {
			std::ifstream save_file(path);
			int actual_version;
			save_file >> actual_version;
			if (save_file_version != actual_version) {
				throw OpenKaiserError("Save file version not matching.");
			}

			save_file >> map_seed_;
			save_file >> year_;

			int numDynasties;
			save_file >> numDynasties;
			for (int i = 0; i < numDynasties; i++) {
				Dynasty dynasty;
				save_file >> dynasty.id >> dynasty.name >> dynasty.human;
				dynasties_.push_back(dynasty);
			}

			int numCountries;
			save_file >> numCountries;
			for (int i = 0; i < numCountries; i++) {
				Country country;
				int x, y;
				save_file >> country.id >> country.name >> country.dynasty_id >> x >> y;
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