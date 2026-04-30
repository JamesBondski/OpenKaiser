export module MapRenderer;

import SDL3;
import WorldState;
import std;
import General;
import ResourceManager;
import UI;
import Config;

using std::uint8_t;

namespace OpenKaiser {

	export enum class RenderMode {
		Rect,
		Image
	};

	export class MapRenderer : public UIElement {
	private:
		std::vector<sdl::Color> country_colors_;
		std::unordered_map<TileType, sdl::TexturePtr> tile_textures_;
		std::unordered_map<BuildingType, sdl::TexturePtr> building_textures_;
		std::unordered_map<TileType, sdl::Color> tile_colors_;

		float tile_size_ = 64;
		RenderMode mode_ = RenderMode::Rect;
		bool show_names_ = true;
		bool scrollable_ = true;

		sdl::FPoint offset_{ 0,0 };

		void DrawTileRect(sdl::Point& offset, Tile& current_tile, int x, int y) {
			switch (current_tile.type) {
			case TileType::Grass:
				sdl::set_render_draw_color(renderer_, { 76, 153, 0, 255 });
				break;
			case TileType::Water:
				sdl::set_render_draw_color(renderer_, { 41, 128, 185, 255 });
				break;
			case TileType::Mountain:
				sdl::set_render_draw_color(renderer_, { 127, 140, 141, 255 });
				break;
			}

			sdl::FRect rect = { offset.x + screen_area_.x + x * tile_size_, offset.y + screen_area_.y + y * tile_size_, tile_size_, tile_size_ };
			sdl::render_fill_rect(renderer_, rect);
		}

		void DrawTileImage(sdl::Point& offset, sdl::TexturePtr& texture, int x, int y) {
			sdl::FRect rect = { offset.x + screen_area_.x + x * tile_size_, offset.y + screen_area_.y + y * tile_size_, tile_size_, tile_size_ };
			sdl::render_texture(renderer_, texture, rect);
		}

		void DrawBorders(sdl::Point& offset, Tile& current_tile, int x, int y)
		{
			if (current_tile.countryId >= 0) {
				auto cc = country_colors_[current_tile.countryId];
				sdl::set_render_draw_color(renderer_, cc);
				if (x > 0 && current_tile.countryId != state_->tile(x - 1, y).countryId) {
					sdl::render_line(renderer_, offset.x + x * tile_size_, offset.y + y * tile_size_, offset.x + x * tile_size_, offset.y + (y + 1) * tile_size_);
				}
				if (y > 0 && current_tile.countryId != state_->tile(x, y - 1).countryId) {
					sdl::render_line(renderer_, offset.x + x * tile_size_, offset.y + y * tile_size_, offset.x + (x + 1) * tile_size_, offset.y + y * tile_size_);
				}
				if (x < state_->tiles().width() - 1 && current_tile.countryId != state_->tile(x + 1, y).countryId) {
					sdl::render_line(renderer_, offset.x + (x + 1) * tile_size_ - 1, offset.y + y * tile_size_, offset.x + (x + 1) * tile_size_ - 1, offset.y + (y + 1) * tile_size_);
				}
				if (y < state_->tiles().height() - 1 && current_tile.countryId != state_->tile(x, y + 1).countryId) {
					sdl::render_line(renderer_, offset.x + x * tile_size_, offset.y + (y + 1) * tile_size_ - 1, offset.x + (x + 1) * tile_size_, offset.y + (y + 1) * tile_size_ - 1);
				}
			}
		}

		void AddTileTexture(TileType type, const std::string& path) {
			tile_textures_.insert(std::pair<TileType, sdl::TexturePtr>(type, resource_manager_->get_image(path)));
		}

		void AddBuildingTexture(BuildingType type, const std::string& path) {
				building_textures_.insert(std::pair<BuildingType, sdl::TexturePtr>(type, resource_manager_->get_image(path)));
			}

	public:
		void set_tile_size(float tile_size) {
				tile_size_ = tile_size;
			}

		float tile_size() const {
				return tile_size_;
			}

		void set_render_mode(RenderMode mode) {
				mode_ = mode;
			}

		RenderMode& render_mode() {
				return mode_;
			}

		void ShowNames() {
			show_names_ = true;
		}

		void HideNames() {
			show_names_ = false;
		}

		bool scrollable() const {
				return scrollable_;
			}

		void set_scrollable(bool scrollable) {
				scrollable_ = scrollable;
			}

		void Init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resource_manager, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller) override {
				UIElement::Init(renderer, resource_manager, state, controller);

				country_colors_ = Config::LoadCountryColors();
				tile_colors_ = Config::LoadTileColors();

				set_screen_area(screen_area_);

				AddTileTexture(TileType::Grass, "data/graphics/tiles/grass.png");
				AddTileTexture(TileType::Water, "data/graphics/tiles/water.png");
				AddTileTexture(TileType::Mountain, "data/graphics/tiles/mountain.png");

				AddBuildingTexture(BuildingType::Castle, "data/graphics/tiles/castle.png");
				AddBuildingTexture(BuildingType::Village, "data/graphics/tiles/village.png");
			AddBuildingTexture(BuildingType::Field, "data/graphics/tiles/field.png");
			AddBuildingTexture(BuildingType::Pasture, "data/graphics/tiles/pasture.png");
			AddBuildingTexture(BuildingType::Palace, "data/graphics/tiles/palace.png");
		}

		void Draw(sdl::Point& offset) override {
			sdl::Point first_tile;
			first_tile.x = (int)offset_.x / (int)tile_size_;
			first_tile.y = (int)offset_.y / (int)tile_size_;

			for (int x = 0; x < (screen_area_.w / tile_size_); x++) {
				for (int y = 0; y < (screen_area_.h / tile_size_); y++) {
					if (x >= (state_->tiles().width()- first_tile.x) || y >= (state_->tiles().height() - first_tile.y)) {
						continue;
					}

					Tile& current_tile = state_->tile(first_tile.x + x, first_tile.y + y);
					if (mode_ == RenderMode::Rect) {
						DrawTileRect(offset, current_tile, x, y);
					}
					if (mode_ == RenderMode::Image) {
						if (current_tile.building != BuildingType::None) {
							DrawTileImage(offset, building_textures_[current_tile.building], x, y);
						}
						else {
							DrawTileImage(offset, tile_textures_[current_tile.type], x, y);
						}
					}

					DrawBorders(offset, current_tile, x, y);
				}
			}

			if (show_names_) {
				for (auto country : state_->countries()) {
					sdl::FPoint target_position = {
						offset.x + screen_area_.x + (country.capital.x - first_tile.x) * tile_size_ + tile_size_ / 2,
						offset.y + screen_area_.y + (country.capital.y - first_tile.y) * tile_size_ + tile_size_ / 2
					};

					sdl::TexturePtr texture;
					if (country.id != state_->current_country_id()) {
						texture = resource_manager_->get_text(country.name, 14, 255, 255, 255);
					}
					else {
						texture = resource_manager_->get_text(country.name, 14, 255, 0, 0);
					}
					sdl::render_texture_centered(renderer_, texture, target_position);
				}
			}
		}

		sdl::SurfacePtr RenderToSurface() {
			auto tiles = state_->tiles();
			std::uint8_t kSaveTileSize = 64;
			sdl::SurfacePtr target_surface = sdl::create_surface(static_cast<size_t>(tiles.width() * kSaveTileSize), static_cast<size_t>(tiles.height() * kSaveTileSize), sdl::PixelFormat::SDL_PIXELFORMAT_RGB24);

			std::unordered_map<TileType, sdl::SurfacePtr> tile_surfaces;
			tile_surfaces[TileType::Grass] = sdl::load_surface("data/graphics/tiles/grass.png");
			tile_surfaces[TileType::Mountain] = sdl::load_surface("data/graphics/tiles/mountain.png");
			tile_surfaces[TileType::Water] = sdl::load_surface("data/graphics/tiles/water.png");

			for (int x = 0; x < tiles.width(); x++) {
				for (int y = 0; y < tiles.height(); y++) {
					Tile& current_tile = tiles(x, y);
					sdl::Rect dstrect{ (x * kSaveTileSize), y * kSaveTileSize, kSaveTileSize, kSaveTileSize };
					sdl::blit_surface(tile_surfaces[current_tile.type], nullptr, target_surface, &dstrect);
				}
			}
			return target_surface;
		}

		void HandleEvent(sdl::Event& event) override {
			if (scrollable_ && event.type == sdl::EventType::KeyDown) {
				switch (event.key.key) {
				case 0x4000004fu:
					offset_.x += tile_size_;
					break;
				case 0x40000050u:
					offset_.x -= tile_size_;
					break;
				case 0x40000051u:
					offset_.y += tile_size_;
					break;
				case 0x40000052u:
					offset_.y -= tile_size_;
					break;
				}

				if (offset_.x < 0) {
					offset_.x = 0;
				}
				if (offset_.y < 0) {
					offset_.y = 0;
				}
			}
			UIElement::HandleEvent(event);
		}
	};

}