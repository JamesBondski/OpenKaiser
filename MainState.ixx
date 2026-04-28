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
		int last_history_count = -1;
		sdl::TexturePtr texture;
		std::unordered_map<TileType, sdl::Color> tileColors;
		std::vector<sdl::Color> countryColors;

		void redraw() {
			Array2D<Tile>& tiles = this->state->tiles();
			sdl::SurfacePtr surface = sdl::create_surface(tiles.width(), tiles.height(), sdl::PixelFormat::SDL_PIXELFORMAT_RGBA32);

			std::uint32_t* pixels = static_cast<std::uint32_t*>(surface->pixels);
			for (int x = 0; x < tiles.width(); x++) {
				for (int y = 0; y < tiles.height(); y++) {
					Tile& tile = tiles(x, y);
					sdl::Color color = this->tileColors[tile.type];

					if (tile.countryId >= 0) {
						color = countryColors[tile.countryId];
					}
					
					pixels[y * tiles.width() + x] = *reinterpret_cast<int*>(&color);
				}
			}
			texture = sdl::create_texture_from_surface(this->renderer, surface);
			sdl::set_texture_scale_mode(texture, sdl::ScaleMode::SDL_SCALEMODE_NEAREST);
		}
	public:
		void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resourceManager, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller) override {
			UIElement::init(renderer, resourceManager, state, controller);

			this->tileColors = Config::load_tile_colors();
			this->countryColors = Config::load_country_colors();
		}

		void draw(sdl::Point& offset) override {
			// If any commands have been executed in the meantime, redraw the mini map.
			if (this->controller->get_history_size() != this->last_history_count) {
				this->redraw();
			}

			sdl::FRect outputArea = this->get_offset_area(offset);
			sdl::render_texture(this->renderer, this->texture, outputArea);
		}
	};

	export class SideBar : public UIElement {
	private:
		std::shared_ptr<MiniMap> miniMap;
		std::shared_ptr<DynamicTextElement> currentPlayerText;
		std::shared_ptr<VerticalStack> stack;
		std::shared_ptr<Padding> padding;

	public:
		void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			UIElement::init(renderer, resources, state, controller);

			this->padding = std::make_shared<Padding>();
			this->padding->set_pad_amount(5);
			this->padding->set_fill(true);
			this->add_child(this->padding);

			this->stack = std::make_shared<VerticalStack>();
			this->padding->add_child(this->stack);

			this->miniMap = std::make_shared<MiniMap>();
			this->stack->add_child(this->miniMap);

			sdl::Color textColor{ 255, 255, 255, 255 };
			std::function<std::string()> textGetter = [this]() { return this->state->countries()[this->state->get_current_country_id()].name; };
			this->currentPlayerText = std::make_shared<DynamicTextElement>(textGetter, (float)12, textColor, true);
			this->currentPlayerText->get_screen_area().h = 30;
			this->stack->add_child(this->currentPlayerText);

			this->set_screen_area(this->screenArea);
		}

		void set_screen_area(sdl::FRect& screenArea) override {
			if (this->miniMap->get_screen_area().h != screenArea.w) {
				sdl::FRect newHeight = this->miniMap->get_screen_area();
				newHeight.h = screenArea.w;
				this->miniMap->set_screen_area(newHeight);
			}

			UIElement::set_screen_area(screenArea);
		}
	};

	export class MainState : public GameState {
	private:
		std::shared_ptr<MapRenderer> mapRenderer;
		std::shared_ptr<MenuManager> menu;
		std::shared_ptr<SideBar> sideBar;

	public:
		void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			GameState::init(renderer, resources, state, controller);

			this->mapRenderer = std::make_shared<MapRenderer>();
			this->add_child(this->mapRenderer);
			this->mapRenderer->set_render_mode(RenderMode::Image);

			this->sideBar = std::make_shared<SideBar>();
			this->add_child(this->sideBar);

			this->menu = std::make_shared<MenuManager>();
			this->add_child(this->menu);

			// Menu
			auto rootItem = this->menu->getRootItem();
			std::shared_ptr<MenuItem> endTurn = std::make_shared<MenuItem>();
			endTurn->name = "endturn";
			endTurn->text = "End (T)urn";
			endTurn->hotkey = sdl::SDLK::T; // T
			endTurn->callback = [this](const std::string itemName) { return this->handle_end_turn(itemName); };
			this->menu->add_item(rootItem, endTurn);

			std::shared_ptr<MenuItem> quit = std::make_shared<MenuItem>();
			quit->name = "quit";
			quit->text = "(Q)uit";
			quit->hotkey = sdl::SDLK::Q; // Q
			quit->callback = [this](const std::string itemName) { return this->handle_quit(itemName); };
			this->menu->add_item(rootItem, quit);

			// Make sure children are sized correctly
			this->set_screen_area(this->screenArea);
		}

		void set_screen_area(sdl::FRect& screenArea) override {
			GameState::set_screen_area(screenArea);

			if (this->mapRenderer && this->menu) {
				const int menuHeight = 200;
				const int sideBarWidth = 260;

				sdl::FRect mapArea(0, 0, this->screenArea.w - sideBarWidth, this->screenArea.h - menuHeight);
				this->mapRenderer->set_screen_area(mapArea);

				sdl::FRect menuArea{ 0, mapArea.y + mapArea.h, this->screenArea.w, menuHeight };
				this->menu->set_screen_area(menuArea);

				sdl::FRect sideBarArea{ mapArea.w, 0, sideBarWidth, this->screenArea.h - menuHeight };
				this->sideBar->set_screen_area(sideBarArea);
			}
		}

		ResultAction handle_quit(const std::string itemName) {
			this->set_next_state("quit");
			return ResultAction::None;
		}

		ResultAction handle_end_turn(const std::string itemName) {
			this->controller->HandleCommand(std::make_unique<EndTurnCommand>());
			return ResultAction::None;
		}
	};
}