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
		std::vector<sdl::Color> country_colors;
		std::unordered_map<TileType, sdl::TexturePtr> tileTextures;
		std::unordered_map<BuildingType, sdl::TexturePtr> buildingTextures;
		sdl::RendererPtr renderer;
		std::shared_ptr<ResourceManager> resourceManager;

		float tileSize = 32;
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
					static_cast<uint8_t>(b),
					255
					});
			}
		}

		void draw_tile_rect(Tile& currentTile, int x, int y) {
			switch (currentTile.type) {
			case TileType::Grass:
				sdl::set_render_draw_color(renderer, { 76, 153, 0, 255 });
				break;
			case TileType::Water:
				sdl::set_render_draw_color(renderer, { 41, 128, 185, 255 });
				break;
			case TileType::Mountain:
				sdl::set_render_draw_color(renderer, { 127, 140, 141, 255 });
				break;
			}

			sdl::FRect rect = { screenArea.x + x * this->tileSize, screenArea.y + y * this->tileSize, this->tileSize, this->tileSize };
			sdl::render_fill_rect(renderer, rect);
		}

		void draw_tile_image(sdl::TexturePtr& texture, int x, int y) {
			sdl::FRect rect = { screenArea.x + x * this->tileSize, screenArea.y + y * this->tileSize, this->tileSize, this->tileSize };
			sdl::render_texture(renderer, texture, rect);
		}

		void draw_borders(WorldState& world, Tile& currentTile, int x, int y)
		{
			// Draw country borders
			if (currentTile.countryId >= 0) {
				auto cc = this->country_colors[currentTile.countryId];
				sdl::set_render_draw_color(renderer, cc);
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

		void add_tile_texture(TileType type, const std::string& path) {
			this->tileTextures.insert(std::pair<TileType, sdl::TexturePtr>(type, this->resourceManager->get_image(path)));
		}

		void add_building_texture(BuildingType type, const std::string& path) {
			this->buildingTextures.insert(std::pair<BuildingType, sdl::TexturePtr>(type, this->resourceManager->get_image(path)));
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

		void init(std::shared_ptr<ResourceManager>& resourceManager, sdl::RendererPtr& renderer, sdl::FRect& screenArea) {
			this->renderer = renderer;
			this->resourceManager = resourceManager;

			this->set_screen_area(screenArea);

			this->add_tile_texture(TileType::Grass, "data/graphics/tiles/grass.png");
			this->add_tile_texture(TileType::Water, "data/graphics/tiles/water.png");
			this->add_tile_texture(TileType::Mountain, "data/graphics/tiles/mountain.png");

			this->add_building_texture(BuildingType::Castle, "data/graphics/tiles/castle.png");
			this->add_building_texture(BuildingType::Village, "data/graphics/tiles/village.png");
			this->add_building_texture(BuildingType::Field, "data/graphics/tiles/field.png");
			this->add_building_texture(BuildingType::Pasture, "data/graphics/tiles/pasture.png");
			this->add_building_texture(BuildingType::Palace, "data/graphics/tiles/palace.png");
		}

		void draw(WorldState& world) {
			for (int x = 0; x < (this->screenArea.w / this->tileSize); x++) {
				for (int y = 0; y < (this->screenArea.h / this->tileSize); y++) {
					if (x >= world.tiles().width() || y >= world.tiles().height()) {
						continue;
					}

					Tile& currentTile = world.tiles()(x, y);
					if (this->mode == RenderMode::Rect) {
						draw_tile_rect(currentTile, x, y);
					}
					if (this->mode == RenderMode::Image) {
						if (currentTile.building != BuildingType::None) {
							draw_tile_image(this->buildingTextures[currentTile.building], x, y);
						}
						else {
							draw_tile_image(this->tileTextures[currentTile.type], x, y);
						}
					}

					draw_borders(world, currentTile, x, y);
				}
			}

			// Render country names
			for (auto country : world.countries()) {
				sdl::FPoint targetPosition = { 
					this->screenArea.x + country.capital.x * this->tileSize + this->tileSize / 2, 
					this->screenArea.y + country.capital.y * this->tileSize + this->tileSize / 2 
				};

				auto texture = this->resourceManager->get_text(country.name, 14, 255, 255, 255);
				sdl::render_texture_centered(this->renderer, texture, targetPosition);
			}
		}
	};

}