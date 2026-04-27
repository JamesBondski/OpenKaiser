export module MapRenderer;

import SDL3;
import WorldState;
import std;
import General;
import ResourceManager;
import UI;

using std::uint8_t;

namespace OpenKaiser {

	export enum class RenderMode {
		Rect,
		Image
	};

	export class MapRenderer : public UIElement {
	private:
		std::vector<sdl::Color> country_colors;
		std::unordered_map<TileType, sdl::TexturePtr> tileTextures;
		std::unordered_map<BuildingType, sdl::TexturePtr> buildingTextures;

		float tileSize = 64;
		RenderMode mode = RenderMode::Rect;

		sdl::FPoint offset{ 0,0 };

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

		void draw_tile_rect(sdl::Point& offset, Tile& currentTile, int x, int y) {
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

			sdl::FRect rect = { offset.x + screenArea.x + x * this->tileSize, offset.y + screenArea.y + y * this->tileSize, this->tileSize, this->tileSize };
			sdl::render_fill_rect(renderer, rect);
		}

		void draw_tile_image(sdl::Point& offset, sdl::TexturePtr& texture, int x, int y) {
			sdl::FRect rect = { offset.x + screenArea.x + x * this->tileSize, offset.y + screenArea.y + y * this->tileSize, this->tileSize, this->tileSize };
			sdl::render_texture(renderer, texture, rect);
		}

		void draw_borders(sdl::Point& offset, Tile& currentTile, int x, int y)
		{
			// Draw country borders
			if (currentTile.countryId >= 0) {
				auto cc = this->country_colors[currentTile.countryId];
				sdl::set_render_draw_color(renderer, cc);
				// Left
				if (x > 0 && currentTile.countryId != this->state->tiles()(x - 1, y).countryId) {
					sdl::render_line(renderer, offset.x + x * this->tileSize, offset.y + y * this->tileSize, offset.x + x * this->tileSize, offset.y + (y + 1) * this->tileSize);
				}
				// Top
				if (y > 0 && currentTile.countryId != this->state->tiles()(x, y - 1).countryId) {
					sdl::render_line(renderer, offset.x + x * this->tileSize, offset.y + y * this->tileSize, offset.x + (x + 1) * this->tileSize, offset.y + y * this->tileSize);
				}
				// Right
				if (x < this->state->tiles().width() - 1 && currentTile.countryId != this->state->tiles()(x + 1, y).countryId) {
					sdl::render_line(renderer, offset.x + (x + 1) * this->tileSize - 1, offset.y + y * this->tileSize, offset.x + (x + 1) * this->tileSize - 1, offset.y + (y + 1) * this->tileSize);
				}
				// Bottom
				if (y < this->state->tiles().height() - 1 && currentTile.countryId != this->state->tiles()(x, y + 1).countryId) {
					sdl::render_line(renderer, offset.x + x * this->tileSize, offset.y + (y + 1) * this->tileSize - 1, offset.x + (x + 1) * this->tileSize, offset.y + (y + 1) * this->tileSize - 1);
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

		void init(std::shared_ptr<ResourceManager>& resourceManager, sdl::RendererPtr& renderer, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController> controller) {
			UIElement::init(renderer, resourceManager, state, controller);

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

		void draw(sdl::Point& offset) override {
			sdl::Point firstTile;
			firstTile.x = (int)this->offset.x / (int)this->tileSize;
			firstTile.y = (int)this->offset.y / (int)this->tileSize;

			for (int x = 0; x < (this->screenArea.w / this->tileSize); x++) {
				for (int y = 0; y < (this->screenArea.h / this->tileSize); y++) {
					if (x >= (this->state->tiles().width()- firstTile.x) || y >= (this->state->tiles().height() - firstTile.y)) {
						continue;
					}

					Tile& currentTile = this->state->tiles()(firstTile.x + x, firstTile.y + y);
					if (this->mode == RenderMode::Rect) {
						draw_tile_rect(offset, currentTile, x, y);
					}
					if (this->mode == RenderMode::Image) {
						if (currentTile.building != BuildingType::None) {
							draw_tile_image(offset, this->buildingTextures[currentTile.building], x, y);
						}
						else {
							draw_tile_image(offset, this->tileTextures[currentTile.type], x, y);
						}
					}

					draw_borders(offset, currentTile, x, y);
				}
			}

			// Render country names
			for (auto country : this->state->countries()) {
				sdl::FPoint targetPosition = { 
					offset.x + this->screenArea.x + (country.capital.x - firstTile.x) * this->tileSize + this->tileSize / 2, 
					offset.y + this->screenArea.y + (country.capital.y - firstTile.y) * this->tileSize + this->tileSize / 2 
				};

				sdl::TexturePtr texture;
				if (country.id != this->state->get_current_country_id()) {
					texture = this->resourceManager->get_text(country.name, 14, 255, 255, 255);
				}
				else {
					texture = this->resourceManager->get_text(country.name, 14, 255, 0, 0);
				}
				sdl::render_texture_centered(this->renderer, texture, targetPosition);
			}
		}

		sdl::SurfacePtr render_to_surface() {
			auto tiles = state->tiles();
			float saveTileSize = 64;
			sdl::SurfacePtr targetSurface = sdl::create_surface(tiles.width() * saveTileSize, tiles.height() * saveTileSize, sdl::PixelFormat::SDL_PIXELFORMAT_RGB24);
			
			std::unordered_map<TileType, sdl::SurfacePtr> tileSurfaces;
			tileSurfaces[TileType::Grass] = sdl::load_surface("data/graphics/tiles/grass.png");
			tileSurfaces[TileType::Mountain] = sdl::load_surface("data/graphics/tiles/mountain.png");
			tileSurfaces[TileType::Water] = sdl::load_surface("data/graphics/tiles/water.png");

			for (int x = 0; x < tiles.width(); x++) {
				for (int y = 0; y < tiles.height(); y++) {
					Tile& currentTile = tiles(x, y);
					sdl::Rect dstrect{ x * saveTileSize, y * saveTileSize, saveTileSize, saveTileSize };
					sdl::blit_surface(tileSurfaces[currentTile.type], nullptr, targetSurface, &dstrect);
				}
			}
			return targetSurface;
		}

		void handle_event(sdl::Event& event) override {
			if (event.type == sdl::EventType::KeyDown) {
				switch (event.key.key) {
				case 0x4000004fu: // Right
					this->offset.x += this->tileSize;
					break;
				case 0x40000050u: // Left
					this->offset.x -= this->tileSize;
					break;
				case 0x40000051u: // Down
					this->offset.y += this->tileSize;
					break;
				case 0x40000052u: // Up
					this->offset.y -= this->tileSize;
					break;
				}

				if (this->offset.x < 0) {
					this->offset.x = 0;
				}
				if (this->offset.y < 0) {
					this->offset.y = 0;
				}
			}
			UIElement::handle_event(event);
		}
	};

}