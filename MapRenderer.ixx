export module MapRenderer;

import SDL3;
import WorldState;
import std;
import General;

using std::uint8_t;

namespace OpenKaiser {

	export class MapRenderer {
	private:
		sdl::FRect screenArea;
		std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> country_colors;

		void load_country_colors() {
			const std::string config_location = "data/config/country_colors.txt";
			std::ifstream config_file(config_location);
			// Check for errors
			if (config_file.bad()) {
				throw OpenKaiserError("Could not open Country Colors file in " + config_location);
			}

			std::string line;
			while (std::getline(config_file, line)) {
				std::stringstream splitter(line);
				int r, g, b;
				splitter >> r >> g >> b;

				if (!splitter.good()) {
					throw OpenKaiserError("Error parsing color: " + line);
				}

				country_colors.push_back({
					static_cast<uint8_t>(r),
					static_cast<uint8_t>(g),
					static_cast<uint8_t>(b)
					});
			}
		}

	public:
		MapRenderer() {
			this->load_country_colors();
		}

		void set_screenArea(sdl::FRect& screenArea) {
			this->screenArea = screenArea;
		}

		void draw(sdl::RendererPtr& renderer, WorldState& world) {
			const float tileSize = 16;
			for (int x = 0; x < (this->screenArea.w / tileSize); x++) {
				for (int y = 0; y < (this->screenArea.h / tileSize); y++) {
					if (x >= world.tiles().width() || y >= world.tiles().height()) {
						continue;
					}

					Tile currentTile = world.tiles()(x, y);

					switch (currentTile.type) {
					case TileType::Grass:
						sdl::set_render_draw_color(renderer, 76, 153, 0, 255);
						break;
					case TileType::Water:
						sdl::set_render_draw_color(renderer, 41, 128, 185, 255);
						break;
					case TileType::Mountain:
						sdl::set_render_draw_color(renderer, 127, 140, 141, 255);
						break;
					}

					sdl::FRect rect = { screenArea.x + x * tileSize, screenArea.y + y * tileSize, tileSize, tileSize };
					sdl::render_fill_rect(renderer, rect);

					// Draw country borders
					if (currentTile.countryId >= 0) {
						auto cc = this->country_colors[currentTile.countryId];
						uint8_t r = std::get<0>(cc);
						sdl::set_render_draw_color(renderer, std::get<0>(cc), std::get<1>(cc), std::get<2>(cc), 255);
						// Left
						if (x > 0 && currentTile.countryId != world.tiles()(x - 1, y).countryId) {
							sdl::render_line(renderer, x * tileSize, y * tileSize, x * tileSize, (y + 1) * tileSize);
						}
						// Top
						if (y > 0 && currentTile.countryId != world.tiles()(x, y - 1).countryId) {
							sdl::render_line(renderer, x * tileSize, y * tileSize, (x + 1) * tileSize, y * tileSize);
						}
						// Right
						if (x < world.tiles().width() - 1 && currentTile.countryId != world.tiles()(x + 1, y).countryId) {
							sdl::render_line(renderer, (x + 1) * tileSize - 1, y * tileSize, (x + 1) * tileSize - 1, (y + 1) * tileSize);
						}
						// Bottom
						if (y < world.tiles().height() - 1 && currentTile.countryId != world.tiles()(x, y + 1).countryId) {
							sdl::render_line(renderer, x * tileSize, (y + 1) * tileSize - 1, (x + 1) * tileSize, (y + 1) * tileSize - 1);
						}
					}
				}
			}

			sdl::render_present(renderer);
		}
	};

}