export module MapRenderer;

import SDL3;
import WorldState;
import std;
import General;
import ResourceManager;
import UI;
import Config;
import Events;

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

		Event<Area&> on_scroll_;

		float tile_size_ = 64;
		float border_size_ = 3;
		RenderMode mode_ = RenderMode::Rect;
		bool show_names_ = true;
		bool scrollable_ = true;

		sdl::FPoint offset_{ 0,0 };

		void DrawTileRect(Tile& current_tile, Coordinates coords) {
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

			sdl::FRect rect = { screen_area_.x + coords.x * tile_size_, screen_area_.y + coords.y * tile_size_, tile_size_, tile_size_ };
			sdl::render_fill_rect(renderer_, rect);
		}

		void DrawTileImage( sdl::TexturePtr& texture, Coordinates coords) {
			sdl::FRect rect = { screen_area_.x + coords.x * tile_size_, screen_area_.y + coords.y * tile_size_, tile_size_, tile_size_ };
			sdl::render_texture(renderer_, texture, rect);
		}

		std::map<Adjacency, Tile> GetTiles(std::array<Coordinates, 4> adjacent_tiles) {
			std::map<Adjacency, Tile> tiles;
			int count = 0;
			for (Coordinates& coords : adjacent_tiles) {
				if (coords.x > 0 && coords.y > 0 && coords.x < state_->tiles().width() && coords.y < state_->tiles().height()) {
					tiles.insert(std::pair<Adjacency, Tile>(static_cast<Adjacency>(count++), state_->tile(coords)));
				}
			}
			return tiles;
		}

		void DrawBorders(Coordinates map_coords, Coordinates draw_coords)
		{
			Tile& current_tile = state_->tile(map_coords);
			if (current_tile.countryId == -1) {
				return;
			}

			sdl::set_render_draw_color(renderer_, country_colors_[current_tile.countryId]);

			std::map<Adjacency, Tile> adjacent_tiles = GetTiles(GetAdjacentTiles(map_coords));
			for (auto adjacent_tile : adjacent_tiles) {
				if (adjacent_tile.second.countryId == current_tile.countryId) {
					continue;
				}

				sdl::FRect tile_rect;
				switch (adjacent_tile.first) {
				case Adjacency::Left:
					tile_rect = { draw_coords.x * tile_size_, draw_coords.y * tile_size_, border_size_, tile_size_ };
					break;
				case Adjacency::Top:
					tile_rect = {  draw_coords.x * tile_size_, draw_coords.y * tile_size_, tile_size_, border_size_ };
					break;
				case Adjacency::Right:
					tile_rect = { draw_coords.x * tile_size_ + tile_size_ - border_size_ - 1, draw_coords.y * tile_size_, border_size_, tile_size_ };
					break;
				case Adjacency::Bottom:
					tile_rect = { draw_coords.x * tile_size_, draw_coords.y * tile_size_ + tile_size_ - border_size_ - 1, tile_size_, border_size_ };
					break;
				}
				sdl::render_fill_rect(renderer_, tile_rect);
			}
		}

		void AddTileTexture(TileType type, const std::string& path) {
			tile_textures_.insert(std::pair<TileType, sdl::TexturePtr>(type, resource_manager_->get_image(path)));
		}

		void AddBuildingTexture(BuildingType type, const std::string& path) {
			building_textures_.insert(std::pair<BuildingType, sdl::TexturePtr>(type, resource_manager_->get_image(path)));
		}

		void Scroll(float delta_x, float delta_y) {
			offset_.x += delta_x;
			offset_.y += delta_y;

			if (offset_.x < 0) {
				offset_.x = 0;
			}
			if (offset_.y < 0) {
				offset_.y = 0;
			}

			Area visible_area = get_visible_area();
			if (visible_area.x + visible_area.w > state_->tiles().width()) {
				offset_.x = (state_->tiles().width() - visible_area.w) * tile_size_;
				visible_area = get_visible_area();
			}
			if (visible_area.y + visible_area.h > state_->tiles().height()) {
				offset_.y = (state_->tiles().height() - visible_area.h) * tile_size_;
				visible_area = get_visible_area();
			}
			on_scroll_.emit(visible_area);
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

		Area get_visible_area() {
			return Area { static_cast<int>(offset_.x / tile_size_), static_cast<int>(offset_.y / tile_size_), static_cast<int>(screen_area_.w / tile_size_) + 1, static_cast<int>(screen_area_.h / tile_size_) + 1 };
		}

		RenderMode& render_mode() {
			return mode_;
		}

		Event<Area&>& on_scroll() {
			return on_scroll_;
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

		void set_screen_area(sdl::FRect& screen_area) override {
			UIElement::set_screen_area(screen_area);
			Scroll(0, 0);
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

		void Draw() override {
			sdl::Point first_tile;
			first_tile.x = (int)offset_.x / (int)tile_size_;
			first_tile.y = (int)offset_.y / (int)tile_size_;

			for (int x = 0; x < (screen_area_.w / tile_size_); x++) {
				for (int y = 0; y < (screen_area_.h / tile_size_); y++) {
					if (x >= (state_->tiles().width() - first_tile.x) || y >= (state_->tiles().height() - first_tile.y)) {
						continue;
					}

					Coordinates map_coords{ first_tile.x + x, first_tile.y + y };
					Coordinates draw_coords{ x, y };
					Tile& current_tile = state_->tile(map_coords);
					if (mode_ == RenderMode::Rect) {
						DrawTileRect(current_tile, draw_coords);
					}
					if (mode_ == RenderMode::Image) {
						if (current_tile.building != BuildingType::None) {
							DrawTileImage(building_textures_[current_tile.building], draw_coords);
						}
						else {
							DrawTileImage(tile_textures_[current_tile.type], draw_coords);
						}
					}

					DrawBorders(map_coords, draw_coords);
				}
			}

			if (show_names_) {
				for (auto country : state_->countries()) {
					sdl::FPoint target_position = {
						screen_area_.x + (country.capital.x - first_tile.x) * tile_size_ + tile_size_ / 2,
						screen_area_.y + (country.capital.y - first_tile.y) * tile_size_ + tile_size_ / 2
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
					Scroll(tile_size_, 0);
					break;
				case 0x40000050u:
					Scroll(-tile_size_, 0);
					break;
				case 0x40000051u:
					Scroll(0, tile_size_);
					break;
				case 0x40000052u:
					Scroll(0, -tile_size_);
					break;
				}
			}
			UIElement::HandleEvent(event);
		}
	};

}