export module WorldGenerator;

import std;
import WorldState;
import General;

namespace OpenKaiser {

	export class MapGenerator {
		virtual void generate(WorldState& state, std::mt19937& gen) = 0;
	};

	export struct WorldConfig {
		int width = 50;
		int height = 35;
		int numCountries = 10;
	};

	export class FloatMapGenerator : public MapGenerator {
	public:
		void generate(WorldState& state, std::mt19937& gen) override {
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);
			auto tiles = state.tiles();

			// Fill initial height map with random values
			Array2D<float> heightMap(tiles.width(), tiles.height());
			for (size_t x = 0; x < tiles.width(); ++x) {
				for (size_t y = 0; y < tiles.height(); ++y) {
					// Fill the edges with water / lowest value
					if(x <= 1 || y <= 1 || x == tiles.width() - 2 || y == tiles.height() - 2) {
						heightMap(x, y) = 0.0f;
					}
					else {
						heightMap(x, y) = dist(gen);
					}
				}
			}

			const int scanWidth = 2;

			// Try to smooth out the height map by averaging each tile with its neighbors
			for (int x = 0; x < tiles.width(); ++x) {
				for (int y = 0; y < tiles.height(); ++y) {
					// Iterate over neighbors and average their heights
					// Let's add weights later
					float value = 0.0f;
					float count = 0.0f;
					for (int nx = x - scanWidth; nx <= x + scanWidth; ++nx) {
						for(int ny = y - scanWidth; ny <= y + scanWidth; ++ny) {
							if (nx >= 0 && nx < tiles.width() && ny >= 0 && ny < tiles.height()) {
								float distance = std::sqrt((nx - x) * (nx - x) + (ny - y) * (ny - y));
								float weight = (distance == 0 ? 2 : 1 / distance);

								count += weight;
								value += heightMap(nx, ny) * weight;
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
		bool tile_is_valid(const Tile& tile) {
			if (tile.type == TileType::Water) {
				return false;
			}
			if (tile.countryId != -1) {
				return false;
			}
			return true;
		}

		bool check_starting_location(WorldState& state, std::pair<int, int> coords) {
			Array2D<Tile>& tiles = state.tiles();
			if (!tile_is_valid(tiles(coords.first, coords.second))) {
				return false;
			}
			// Left
			if (coords.first > 0 && !tile_is_valid(tiles(coords.first - 1, coords.second))) {
				return false;
			}
			// Top
			if (coords.second > 0 && !tile_is_valid(tiles(coords.first, coords.second - 1))) {
				return false;
			}
			// Right
			if (coords.first < state.tiles().width() && !tile_is_valid(tiles(coords.first + 1, coords.second))) {
				return false;
			}
			// Bottom
			if (coords.second < state.tiles().height() && !tile_is_valid(tiles(coords.first, coords.second + 1))) {
				return false;
			}

			return true;
		}

		std::pair<int, int> get_starting_location(WorldState& state, std::mt19937& gen) {
			std::uniform_int_distribution<int> x_dist(0, (int)state.tiles().width() - 1);
			std::uniform_int_distribution<int> y_dist(0, (int)state.tiles().height() - 1);

			auto coords = std::pair(x_dist(gen), y_dist(gen));
			int count = 0;
			while (!this->check_starting_location(state, coords)) {
				coords = std::pair(x_dist(gen), y_dist(gen));
				count++;

				if (count > 1000) {
					throw OpenKaiserError("Could not find a valid starting location.");
				}
			}

			return coords;
		}
	public:
		WorldState generate(const WorldConfig& config) {
			WorldState state = WorldState(config.width, config.height);
			std::random_device rd;
			std::mt19937 gen(rd());

			// Generate seed
			std::uniform_int_distribution<int> seed_dist;
			int seed = seed_dist(gen);
			gen.seed(seed);
			std::cout << "Map seed: " << seed << std::endl;
			state.setMapSeed(seed);

			// Generate map
			FloatMapGenerator mapGen;
			mapGen.generate(state, gen);

			// Place countries
			
			for (int i = 0; i < config.numCountries; i++) {
				std::pair<int, int> coords = get_starting_location(state, gen);
				
				// Assign initial tiles
				state.tiles()(coords.first, coords.second).countryId = i;
				state.tiles()(coords.first - 1, coords.second).countryId = i;
				state.tiles()(coords.first, coords.second - 1).countryId = i;
				state.tiles()(coords.first + 1, coords.second).countryId = i;
				state.tiles()(coords.first, coords.second + 1).countryId = i;
			}
			return state;
		}
	};
}