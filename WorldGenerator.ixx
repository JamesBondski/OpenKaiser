export module WorldGenerator;

import std;
import WorldState;
import General;

namespace OpenKaiser {

	export class MapGenerator {
		virtual void Generate(std::shared_ptr<WorldState>& state, std::mt19937& gen) = 0;
	};

	export struct WorldConfig {
		int width = 50;
		int height = 50;
		int num_countries = 25;
	};

	export class FloatMapGenerator : public MapGenerator {
	public:
		void Generate(std::shared_ptr<WorldState>& state, std::mt19937& gen) override {
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);
			Array2D<Tile>& tiles = state->tiles();

			Array2D<float> height_map(tiles.width(), tiles.height());
			for (size_t x = 0; x < tiles.width(); ++x) {
				for (size_t y = 0; y < tiles.height(); ++y) {
					if(x <= 1 || y <= 1 || x == tiles.width() - 2 || y == tiles.height() - 2) {
						height_map(x, y) = 0.0f;
					}
					else {
						height_map(x, y) = dist(gen);
					}
				}
			}

			const int kScanWidth = 2;

			for (int x = 0; x < tiles.width(); ++x) {
				for (int y = 0; y < tiles.height(); ++y) {
					float value = 0.0f;
					float count = 0.0f;
					for (int nx = x - kScanWidth; nx <= x + kScanWidth; ++nx) {
						for(int ny = y - kScanWidth; ny <= y + kScanWidth; ++ny) {
							if (nx >= 0 && nx < tiles.width() && ny >= 0 && ny < tiles.height()) {
								float distance = static_cast<float>(std::sqrt((nx - x) * (nx - x) + (ny - y) * (ny - y)));
								float weight = (distance == 0 ? 2 : 1 / distance);

								count += weight;
								value += height_map(nx, ny) * weight;
							}
						}
					}

					if (count > 0) {
						value = value / count;
						if (value <= 0.41 || x == 0 || y == 0 || x == tiles.width() - 1 || y == tiles.height() - 1) {
							tiles(x, y).type = TileType::Water;
						}
						else if (value <= 0.62) {
							tiles(x, y).type = TileType::Grass;
						}
						else {
							tiles(x, y).type = TileType::Mountain;
						}
					}
				}
			}
		}
	};

	export class WorldGenerator {
	private:
		bool TileIsValid(const Tile& tile) {
			if (tile.type == TileType::Water) {
				return false;
			}
			if (tile.countryId != -1) {
				return false;
			}
			return true;
		}

		bool CheckStartingLocation(std::shared_ptr<WorldState> state, std::pair<int, int> coords) {
			Array2D<Tile>& tiles = state->tiles();
			if (!TileIsValid(tiles(coords.first, coords.second))) {
				return false;
			}
			if (coords.first > 0 && !TileIsValid(tiles(coords.first - 1, coords.second))) {
				return false;
			}
			if (coords.second > 0 && !TileIsValid(tiles(coords.first, coords.second - 1))) {
				return false;
			}
			if (coords.first < state->tiles().width() && !TileIsValid(tiles(coords.first + 1, coords.second))) {
				return false;
			}
			if (coords.second < state->tiles().height() && !TileIsValid(tiles(coords.first, coords.second + 1))) {
				return false;
			}

			return true;
		}

		std::pair<int, int> GetStartingLocation(std::shared_ptr<WorldState> state, std::mt19937& gen) {
			std::uniform_int_distribution<int> x_dist(0, (int)state->tiles().width() - 1);
			std::uniform_int_distribution<int> y_dist(0, (int)state->tiles().height() - 1);

			auto coords = std::pair(x_dist(gen), y_dist(gen));
			int count = 0;
			while (!CheckStartingLocation(state, coords)) {
				coords = std::pair(x_dist(gen), y_dist(gen));
				count++;

				if (count > 1000) {
					throw OpenKaiserError("Could not find a valid starting location.");
				}
			}

			return coords;
		}

		void GenerateCountries(std::shared_ptr<WorldState> state, int num_countries, std::mt19937& gen) {
			std::ifstream country_names("data/config/country_names.txt");
			std::string line;
			std::uniform_int_distribution build_dist(2, 5);

			int count = 0;
			while (std::getline(country_names, line) && count < num_countries) {
				Dynasty new_dynasty;
				new_dynasty.id = count;
				new_dynasty.name = line;
				new_dynasty.human = (count == 0);
				state->dynasties().push_back(new_dynasty);

				Country new_country;
				new_country.id = count++;
				new_country.name = line;
				new_country.resources[ResourceType::Gold] = 1000;
				new_country.resources[ResourceType::Wheat] = 0;
				new_country.resources[ResourceType::Livestock] = 0;
				new_country.dynasty_id = new_dynasty.id;
				new_country.builds_left = build_dist(gen);
				state->countries().push_back(new_country);
			}
		}

		void InitTile(Tile& tile, int country_id) {
			tile.countryId = country_id;
			if (tile.type == TileType::Grass) {
				tile.building = BuildingType::Field;
			} else if (tile.type == TileType::Mountain) {
				tile.building = BuildingType::Pasture;
			}
		}

	public:

		std::shared_ptr<WorldState> Generate(const WorldConfig& config, std::mt19937& gen) {
			std::shared_ptr<WorldState> state(new WorldState(config.width, config.height));

			std::uniform_int_distribution<int> seed_dist;
			int seed = seed_dist(gen);
			gen.seed(seed);
			std::cout << "Map seed: " << seed << std::endl;
			state->set_map_seed(seed);

			FloatMapGenerator map_gen;
			map_gen.Generate(state, gen);

			GenerateCountries(state, config.num_countries, gen);

			for (int i = 0; i < config.num_countries; i++) {
				std::pair<int, int> coords = GetStartingLocation(state, gen);

				Tile& center_tile = state->tile(coords.first, coords.second);
				center_tile.countryId = i;
				center_tile.building = BuildingType::Castle;

				Tile& left_tile = state->tile(coords.first - 1, coords.second);
				left_tile.countryId = i;
				left_tile.building = BuildingType::Village;
				left_tile.population = 100;

				InitTile(state->tile(coords.first, coords.second - 1), i);
				InitTile(state->tile(coords.first + 1, coords.second), i);
				InitTile(state->tile(coords.first, coords.second + 1), i);

				state->country(i).capital = { coords.first, coords.second };
			}
			return state;
		}
	};
}