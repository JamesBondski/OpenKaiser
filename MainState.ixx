export module MainState;

import std;
import General;
import UI;
import MapRenderer;
import Menu;
import SDL3;
import ResourceManager;
import WorldState;
import GameController;
import Menu;
import Config;
import Events;

namespace OpenKaiser {
	const float kMenuHeight = 200;
	const float kSideBarWidth = 260;
	const float kSideBarPadding = 5;

	export class MiniMap : public Image {
	private:
		int last_history_count_ = -1;
		std::unordered_map<TileType, sdl::Color> tile_colors_;
		std::vector<sdl::Color> country_colors_;
		int last_redraw_count_ = -1;
		Area visible_map_area_;
		std::weak_ptr<MapRenderer> map_renderer_;

		Connection handle_map_scroll;

		void Redraw() {
			Array2D<Tile>& tiles = state_->tiles();
			sdl::SurfacePtr surface = sdl::create_surface(tiles.width(), tiles.height(), sdl::PixelFormat::SDL_PIXELFORMAT_RGBA32);

			std::uint32_t* pixels = static_cast<std::uint32_t*>(surface->pixels);
			for (int x = 0; x < tiles.width(); x++) {
				for (int y = 0; y < tiles.height(); y++) {
					Tile& tile = tiles(x, y);
					sdl::Color color = tile_colors_[tile.type];

					if (tile.countryId >= 0) {
						color = country_colors_[tile.countryId];
					}

					pixels[y * tiles.width() + x] = *reinterpret_cast<int*>(&color);
				}
			}
			texture_ = sdl::create_texture_from_surface(renderer_, surface);
			sdl::set_texture_scale_mode(texture_, sdl::ScaleMode::SDL_SCALEMODE_NEAREST);
		}
	public:
		void Init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resource_manager, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller) override {
			Image::Init(renderer, resource_manager, state, controller);

			tile_colors_ = Config::LoadTileColors();
			country_colors_ = Config::LoadCountryColors();
		}

		void Draw() override {
			Image::Draw();

			// Draw visible area
			float ratio = screen_area_.w / static_cast<float>(texture_->w);
			sdl::set_render_draw_color(renderer_, { 255, 255, 255, 255 });
			sdl::FRect draw_rect{screen_area_.x + visible_map_area_.x * ratio,  screen_area_.y + visible_map_area_.y * ratio, visible_map_area_.w * ratio, visible_map_area_.h * ratio};
			sdl::render_rect(renderer_, draw_rect);
		}

		void Update(float passed_time) override {
			if(!texture_ || controller_->command_count() != last_redraw_count_) {
				last_redraw_count_ = controller_->command_count();
				Redraw();
			}
		}

		void UpdateVisibleMapArea(Area& visible_area) {
			visible_map_area_ = visible_area;
		}

		void AttachToMapRenderer(std::shared_ptr<MapRenderer>& map_renderer) {
			map_renderer_ = map_renderer;
			visible_map_area_ = map_renderer->get_visible_area();
			handle_map_scroll = map_renderer->on_scroll().subscribe(this, &MiniMap::UpdateVisibleMapArea);
		}
	};

	export class SideBar : public VerticalStack {
	private:
		std::shared_ptr<MiniMap> mini_map_;

	public:
		void Init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			UIElement::Init(renderer, resources, state, controller);

			mini_map_ = std::make_shared<MiniMap>();
			mini_map_->set_layout({kSideBarWidth - 2 * kSideBarPadding, LayoutMode::Fixed, 1, LayoutMode::Ratio });
			AddChild(mini_map_);

			sdl::Color text_color{ 255, 255, 255, 255 };
			std::function<std::string()> text_getter = [this]() { return "Country: " + state_->current_country().name; };
			std::shared_ptr<DynamicTextElement> current_player_text = std::make_shared<DynamicTextElement>(text_getter, (float)14, text_color, false);
			current_player_text->set_layout({0, LayoutMode::Fill, 25, LayoutMode::Fixed});
			AddChild(current_player_text);

			text_getter = [this]() { return "Year: " + std::to_string(state_->year()); };
			std::shared_ptr<DynamicTextElement> current_year_text = std::make_shared<DynamicTextElement>(text_getter, (float)14, text_color, false);
			current_year_text->set_layout({ 0, LayoutMode::Fill, 25, LayoutMode::Fixed });
			AddChild(current_year_text);

			text_getter = [this]() { return "Gold: " + std::to_string(state_->current_country().resources[ResourceType::Gold]); };
			std::shared_ptr<DynamicTextElement> gold_text = std::make_shared<DynamicTextElement>(text_getter, (float)14, text_color, false);
			gold_text->set_layout({ 0, LayoutMode::Fill, 25, LayoutMode::Fixed });
			AddChild(gold_text);

			set_screen_area(screen_area_);
		}

		void set_screen_area(sdl::FRect& screen_area) override {
			if (mini_map_ && mini_map_->screen_area().h != screen_area.w) {
				sdl::FRect new_height = mini_map_->screen_area();
				new_height.h = screen_area.w;
				mini_map_->set_screen_area(new_height);
			}

			VerticalStack::set_screen_area(screen_area);
		}

		void AttachToMapRenderer(std::shared_ptr<MapRenderer>& map_renderer) {
			mini_map_->AttachToMapRenderer(map_renderer);
		}
	};

	export class MainState : public GameState {
	private:
		std::shared_ptr<MapRenderer> map_renderer_;
		std::shared_ptr<MenuManager> menu_;
		std::shared_ptr<Padded<SideBar>> side_bar_;
		std::shared_ptr<VerticalStack> left_stack_;

		ScopedConnections handlers_;

	public:
		void Init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			GameState::Init(renderer, resources, state, controller);

			left_stack_ = std::make_shared<VerticalStack>();
			AddChild(left_stack_);
			left_stack_->set_layout({ 0, LayoutMode::Fill, 0, LayoutMode::Fill });

			side_bar_ = std::make_shared<Padded<SideBar>>();
			AddChild(side_bar_);
			side_bar_->set_pad_amount(kMenuHeight);
			side_bar_->set_layout({ kSideBarWidth, LayoutMode::Fixed, 0, LayoutMode::Fill });

			map_renderer_ = std::make_shared<MapRenderer>();
			left_stack_->AddChild(map_renderer_);
			map_renderer_->set_render_mode(RenderMode::Image);
			map_renderer_->set_layout({ 0, LayoutMode::Fill, 0, LayoutMode::Fill });
			side_bar_->AttachToMapRenderer(map_renderer_);

			menu_ = std::make_shared<MenuManager>();
			left_stack_->AddChild(menu_);
			menu_->set_layout({0, LayoutMode::Fill, kMenuHeight, LayoutMode::Fixed });

			auto root_item = menu_->get_root_item();

			std::shared_ptr<MenuItem> build_menu = std::make_shared<MenuItem>();
			build_menu->text = "(B)uild";
			build_menu->name = "build";
			build_menu->hotkey = sdl::SDLK::B;
			menu_->add_item(root_item, build_menu);

			std::shared_ptr<MenuItem> build_field = std::make_shared<MenuItem>();
			build_field->name = "buildfield";
			build_field->text = "(F)ield";
			build_field->hotkey = sdl::SDLK::F;
			menu_->add_item(build_menu, build_field);

			std::shared_ptr<MenuItem> end_turn = std::make_shared<MenuItem>();
			end_turn->name = "endturn";
			end_turn->text = "End (T)urn";
			end_turn->hotkey = sdl::SDLK::T;
			handlers_ += end_turn->on_action.subscribe(this, &MainState::HandleEndTurn);
			menu_->add_item(root_item, end_turn);

			std::shared_ptr<MenuItem> quit = std::make_shared<MenuItem>();
			quit->name = "quit";
			quit->text = "(Q)uit";
			quit->hotkey = sdl::SDLK::Q;
			handlers_ += quit->on_action.subscribe(this, &MainState::HandleQuit);
			menu_->add_item(root_item, quit);

			handlers_ += this->controller_->on_start_human_turn().subscribe(this, &MainState::HandleStartHumanTurn);

			set_screen_area(screen_area_);
		}

		void HandleStartHumanTurn(std::uint16_t countryId) {
			
		}

		void HandleQuit(const std::string& item_name) {
			set_next_state("quit");
		}

		void HandleEndTurn(const std::string& item_name) {
			controller_->EndTurn();
		}
	};
}