export module MapGenerator;

import std;
import WorldState;

namespace OpenKaiser {

	export class MapGenerator {
		virtual void generate(WorldState& state) = 0;
	};

	export class RandomMapGenerator : public MapGenerator {
		public:
		void generate(WorldState& state) override {
			std::mt19937 gen;
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);

			auto tiles = state.tiles();
			for (size_t x = 0; x < tiles.extent(0); ++x) {
				for (size_t y = 0; y < tiles.extent(1); ++y) {
					float value = dist(gen);

					if (value <= 0.3) {
						tiles[x, y].type = TileType::Water;
					}
					else if (value <= 0.5) {
						tiles[x, y].type = TileType::Mountain;
					}
					else {
						tiles[x, y].type = TileType::Grass;
					}
				}
			}
		}
	};
}