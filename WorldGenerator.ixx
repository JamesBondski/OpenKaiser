export module WorldGenerator;

import std;
import WorldState;

namespace OpenKaiser {

	export class MapGenerator {
		virtual void generate(WorldState& state, std::mt19937& gen) = 0;
	};

	export struct WorldConfig {
		int width = 40;
		int height = 30;
		int numCountries = 10;
	};

	export class FloatMapGenerator : public MapGenerator {
	public:
		void generate(WorldState& state, std::mt19937& gen) override {
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);
			auto tiles = state.tiles();

			// Fill initial height map with random values
			std::vector<float> heightMap(tiles.size());
			std::mdspan<float, std::dextents<size_t, 2>> heightMapSpan(heightMap.data(), tiles.extent(0), tiles.extent(1));
			for (size_t x = 0; x < tiles.extent(0); ++x) {
				for (size_t y = 0; y < tiles.extent(1); ++y) {
					// Fill the edges with water / lowest value
					if(x <= 1 || y <= 1 || x == tiles.extent(0) - 2 || y == tiles.extent(1) - 2) {
						heightMapSpan[std::array{ x, y }] = 0.0f;
					}
					else {
						heightMapSpan[std::array{ x, y }] = dist(gen);
					}
				}
			}

			const int scanWidth = 2;

			// Try to smooth out the height map by averaging each tile with its neighbors
			for (int x = 0; x < tiles.extent(0); ++x) {
				for (int y = 0; y < tiles.extent(1); ++y) {
					// Iterate over neighbors and average their heights
					// Let's add weights later
					float value = 0.0f;
					float count = 0.0f;
					for (int nx = x - scanWidth; nx <= x + scanWidth; ++nx) {
						for(int ny = y - scanWidth; ny <= y + scanWidth; ++ny) {
							if (nx >= 0 && nx < tiles.extent(0) && ny >= 0 && ny < tiles.extent(1)) {
								float distance = std::sqrt((nx - x) * (nx - x) + (ny - y) * (ny - y));
								float weight = (distance == 0 ? 2 : 1 / distance);

								count += weight;
								value += heightMapSpan[std::array{ nx, ny }] * weight;
							}
						}
					}

					if (count > 0) {
						value = value / count;
						if (value <= 0.41 || x == 0 || y == 0 || x == tiles.extent(0) - 1 || y == tiles.extent(1) - 1) {
							tiles[std::array{ x, y }].type = TileType::Water;
						}
						else if (value <= 0.62) {
							tiles[std::array{ x, y }].type = TileType::Grass;
						}
						else {
							tiles[std::array{ x, y }].type = TileType::Mountain;
						}
					}
				}
			}
		}
	};

	export class WorldGenerator {
	private:
		bool check_starting_location(WorldState& state, std::pair<int, int> coords) {
			return true;
		}

		std::pair<int, int> get_starting_location(WorldState& state, std::mt19937& gen) {
			std::uniform_int_distribution<int> x_dist(0, (int)state.tiles().extent(0) - 1);
			std::uniform_int_distribution<int> y_dist(0, (int)state.tiles().extent(1) - 1);

			auto coords = std::pair(x_dist(gen), y_dist(gen));
			while (!this->check_starting_location(state, coords)) {
				coords = std::pair(x_dist(gen), y_dist(gen));
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
				
				Tile& tile = state.tiles()[std::array{ (size_t)coords.first, (size_t)coords.second }];
				tile.countryId = i;
			}
			return state;
		}
	};
}