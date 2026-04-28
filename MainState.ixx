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

namespace OpenKaiser {
	export class MiniMap : public UIElement {
	private:
		int last_history_count_ = -1;
		sdl::TexturePtr texture_;
		std::unordered_map<TileType, sdl::Color> tile_colors_;
		std::vector<sdl::Color> country_colors_;

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
			UIElement::Init(renderer, resource_manager, state, controller);

			tile_colors_ = Config::LoadTileColors();
			country_colors_ = Config::LoadCountryColors();
		}

		void Draw(sdl::Point& offset) override {
				if (controller_->history_size() != last_history_count_) {
					Redraw();
				}

				sdl::FRect output_area = GetOffsetArea(offset);
				sdl::render_texture(renderer_, texture_, output_area);
			}
	};

	export class SideBar : public UIElement {
	private:
		std::shared_ptr<MiniMap> mini_map_;
		std::shared_ptr<VerticalStack> stack_;
		std::shared_ptr<Padding> padding_;

	public:
		void Init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			UIElement::Init(renderer, resources, state, controller);

				padding_ = std::make_shared<Padding>();
				padding_->set_pad_amount(5);
				padding_->set_fill(true);
				AddChild(padding_);

			stack_ = std::make_shared<VerticalStack>();
			padding_->AddChild(stack_);

			mini_map_ = std::make_shared<MiniMap>();
			stack_->AddChild(mini_map_);

			sdl::Color text_color{ 255, 255, 255, 255 };
			std::function<std::string()> text_getter = [this]() { return "Country: " + state_->countries()[state_->current_country_id()].name; };
			std::shared_ptr<DynamicTextElement> current_player_text = std::make_shared<DynamicTextElement>(text_getter, (float)14, text_color, false);
			current_player_text->screen_area().h = 25;
			stack_->AddChild(current_player_text);

			text_getter = [this]() { return "Year: " + std::to_string(state_->year()); };
			std::shared_ptr<DynamicTextElement> current_year_text = std::make_shared<DynamicTextElement>(text_getter, (float)14, text_color, false);
			current_year_text->screen_area().h = 25;
			stack_->AddChild(current_year_text);

			text_getter = [this]() { return "Gold: " + std::to_string(state_->countries()[state_->current_country_id()].gold); };
			std::shared_ptr<DynamicTextElement> gold_text = std::make_shared<DynamicTextElement>(text_getter, (float)14, text_color, false);
			gold_text->screen_area().h = 25;
			stack_->AddChild(gold_text);

			set_screen_area(screen_area_);
		}

		void set_screen_area(sdl::FRect& screen_area) override {
				if (mini_map_->screen_area().h != screen_area.w) {
					sdl::FRect new_height = mini_map_->screen_area();
					new_height.h = screen_area.w;
					mini_map_->set_screen_area(new_height);
				}

				UIElement::set_screen_area(screen_area);
			}
	};

	export class MainState : public GameState {
	private:
		std::shared_ptr<MapRenderer> map_renderer_;
		std::shared_ptr<MenuManager> menu_;
		std::shared_ptr<SideBar> side_bar_;

	public:
		void Init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			GameState::Init(renderer, resources, state, controller);

			map_renderer_ = std::make_shared<MapRenderer>();
				AddChild(map_renderer_);
				map_renderer_->set_render_mode(RenderMode::Image);

			side_bar_ = std::make_shared<SideBar>();
			AddChild(side_bar_);

			menu_ = std::make_shared<MenuManager>();
			AddChild(menu_);

			auto root_item = menu_->get_root_item();
			std::shared_ptr<MenuItem> end_turn = std::make_shared<MenuItem>();
			end_turn->name = "endturn";
			end_turn->text = "End (T)urn";
			end_turn->hotkey = sdl::SDLK::T;
			end_turn->callback = [this](const std::string item_name) { return HandleEndTurn(item_name); };
			menu_->add_item(root_item, end_turn);

			std::shared_ptr<MenuItem> quit = std::make_shared<MenuItem>();
			quit->name = "quit";
			quit->text = "(Q)uit";
			quit->hotkey = sdl::SDLK::Q;
			quit->callback = [this](const std::string item_name) { return HandleQuit(item_name); };
			menu_->add_item(root_item, quit);

			set_screen_area(screen_area_);
		}

		void set_screen_area(sdl::FRect& screen_area) override {
				GameState::set_screen_area(screen_area);

				if (map_renderer_ && menu_) {
					const int kMenuHeight = 200;
					const int kSideBarWidth = 260;

					sdl::FRect map_area(0, 0, screen_area_.w - kSideBarWidth, screen_area_.h - kMenuHeight);
					map_renderer_->set_screen_area(map_area);

					sdl::FRect menu_area{ 0, map_area.y + map_area.h, screen_area_.w, kMenuHeight };
					menu_->set_screen_area(menu_area);

					sdl::FRect side_bar_area{ map_area.w, 0, kSideBarWidth, screen_area_.h - kMenuHeight };
					side_bar_->set_screen_area(side_bar_area);
				}
			}

		ResultAction HandleQuit(const std::string item_name) {
				set_next_state("quit");
				return ResultAction::None;
			}

		ResultAction HandleEndTurn(const std::string item_name) {
			controller_->HandleCommand(std::make_unique<EndTurnCommand>());
			return ResultAction::None;
		}
	};
}