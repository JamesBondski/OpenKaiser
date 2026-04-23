export module MapRenderer;

import SDL3;
import WorldState;
import std;
import General;
import ImageManager;

using std::uint8_t;

namespace OpenKaiser {

	export enum class RenderMode {
		Rect,
		Image
	};

	export class MapRenderer {
	private:
		sdl::FRect screenArea;
		std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> country_colors;
		std::unordered_map<TileType, sdl::TexturePtr> tileTextures;

		float tileSize = 16;
		RenderMode mode = RenderMode::Rect;

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

		void draw_tile_rect(sdl::RendererPtr& renderer, Tile& currentTile, int x, int y) {
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

			sdl::FRect rect = { screenArea.x + x * this->tileSize, screenArea.y + y * this->tileSize, this->tileSize, this->tileSize };
			sdl::render_fill_rect(renderer, rect);
		}

		void draw_tile_image(sdl::RendererPtr& renderer, Tile& currentTile, int x, int y) {
			sdl::FRect rect = { screenArea.x + x * this->tileSize, screenArea.y + y * this->tileSize, this->tileSize, this->tileSize };
			sdl::render_texture(renderer, this->tileTextures[currentTile.type], rect);
		}

		void draw_borders(sdl::RendererPtr& renderer, WorldState& world, Tile& currentTile, int x, int y)
		{
			// Draw country borders
			if (currentTile.countryId >= 0) {
				auto cc = this->country_colors[currentTile.countryId];
				uint8_t r = std::get<0>(cc);
				sdl::set_render_draw_color(renderer, std::get<0>(cc), std::get<1>(cc), std::get<2>(cc), 255);
				// Left
				if (x > 0 && currentTile.countryId != world.tiles()(x - 1, y).countryId) {
					sdl::render_line(renderer, x * this->tileSize, y * this->tileSize, x * this->tileSize, (y + 1) * this->tileSize);
				}
				// Top
				if (y > 0 && currentTile.countryId != world.tiles()(x, y - 1).countryId) {
					sdl::render_line(renderer, x * this->tileSize, y * this->tileSize, (x + 1) * this->tileSize, y * this->tileSize);
				}
				// Right
				if (x < world.tiles().width() - 1 && currentTile.countryId != world.tiles()(x + 1, y).countryId) {
					sdl::render_line(renderer, (x + 1) * this->tileSize - 1, y * this->tileSize, (x + 1) * this->tileSize - 1, (y + 1) * this->tileSize);
				}
				// Bottom
				if (y < world.tiles().height() - 1 && currentTile.countryId != world.tiles()(x, y + 1).countryId) {
					sdl::render_line(renderer, x * this->tileSize, (y + 1) * this->tileSize - 1, (x + 1) * this->tileSize, (y + 1) * this->tileSize - 1);
				}
			}
		}

		void add_tile_texture(ImageManager& imageManager, sdl::RendererPtr& renderer, TileType type, const std::string& path) {
			this->tileTextures.insert(std::pair<TileType, sdl::TexturePtr>(type, imageManager.get_image(renderer, path)));
		}

	public:
		MapRenderer() {
			this->load_country_colors();
		}

		void set_screen_area(sdl::FRect& screenArea) {
			this->screenArea = screenArea;
		}

		void set_tile_size(float tileSize) {
			this->tileSize = tileSize;
		}

		float get_tile_size() {
			return this->tileSize;
		}

		void set_render_mode(RenderMode mode) {
			this->mode = mode;
		}

		RenderMode& get_render_mode() {
			return this->mode;
		}

		void init(ImageManager& imageManager, sdl::RendererPtr& renderer, sdl::FRect& screenArea) {
			this->set_screen_area(screenArea);

			this->add_tile_texture(imageManager, renderer, TileType::Grass, "data/graphics/tiles/grass.png");
			this->add_tile_texture(imageManager, renderer, TileType::Water, "data/graphics/tiles/water.png");
			this->add_tile_texture(imageManager, renderer, TileType::Mountain, "data/graphics/tiles/mountain.png");
		}

		void draw(sdl::RendererPtr& renderer, WorldState& world) {
			for (int x = 0; x < (this->screenArea.w / this->tileSize); x++) {
				for (int y = 0; y < (this->screenArea.h / this->tileSize); y++) {
					if (x >= world.tiles().width() || y >= world.tiles().height()) {
						continue;
					}

					Tile& currentTile = world.tiles()(x, y);
					if (this->mode == RenderMode::Rect) {
						draw_tile_rect(renderer, currentTile, x, y);
					}
					if (this->mode == RenderMode::Image) {
						draw_tile_image(renderer, currentTile, x, y);
					}

					draw_borders(renderer, world, currentTile, x, y);
				}
			}

			sdl::render_present(renderer);
		}
	};

}