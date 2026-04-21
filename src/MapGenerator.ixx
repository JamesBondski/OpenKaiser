export module MapGenerator;

import std;
import WorldState;

namespace OpenKaiser {

	export class MapGenerator {
		virtual void generate(WorldState& state) = 0;
	};

	export class FloatMapGenerator : public MapGenerator {
	public:
		void generate(WorldState& state) override {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);
			auto tiles = state.tiles();

			// Fill initial height map with random values
			std::vector<float> heightMap(tiles.size());
			std::mdspan heightMapSpan(heightMap.data(), tiles.extent(0), tiles.extent(1));
			for (size_t x = 0; x < tiles.extent(0); ++x) {
				for (size_t y = 0; y < tiles.extent(1); ++y) {
					// Fill the edges with water / lowest value
					if(x == 0 || y == 0 || x == tiles.extent(0) - 1 || y == tiles.extent(1) - 1) {
						heightMapSpan[x, y] = 0.0f;
					}
					else {
						heightMapSpan[x, y] = dist(gen);
					}
				}
			}

			const int scanWidth = 1;

			// Try to smooth out the height map by averaging each tile with its neighbors
			for (int x = 0; x < tiles.extent(0); ++x) {
				for (int y = 0; y < tiles.extent(1); ++y) {
					// Iterate over neighbors and average their heights
					// Let's add weights later
					float value = 0.0f;
					int count = 0;
					for (int nx = x - scanWidth; nx <= x + scanWidth; ++nx) {
						for(int ny = y - scanWidth; ny <= y + scanWidth; ++ny) {
							if (nx >= 0 && nx < tiles.extent(0) && ny >= 0 && ny < tiles.extent(1)) {
								count++;
								value += heightMapSpan[nx, ny];
							}
						}
					}

					if (count > 0) {
						value = value / count;
						if (value <= 0.35 || x == 0 || y == 0 || x == tiles.extent(0) - 1 || y == tiles.extent(1) - 1) {
							tiles[x, y].type = TileType::Water;
						}
						else if (value <= 0.67) {
							tiles[x, y].type = TileType::Grass;
						}
						else {
							tiles[x, y].type = TileType::Mountain;
						}
					}
				}
			}
		}
	};
}