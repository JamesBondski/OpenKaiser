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

namespace OpenKaiser {
	export class SideBar : public UIElement {
	private:
		std::shared_ptr<MapRenderer> miniMap;
		std::shared_ptr<DynamicTextElement> currentPlayerText;
		const float padding = 5;

	public:
		void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			UIElement::init(renderer, resources, state, controller);

			this->miniMap = std::make_shared<MapRenderer>();
			this->add_child(this->miniMap);
			this->miniMap->set_render_mode(RenderMode::Image);
			this->miniMap->hide_names();

			SDL_Color textColor{ 255, 255, 255, 255 };
			std::function<std::string()> textGetter = [this]() { return this->state->countries()[this->state->get_current_country_id()].name; };
			this->currentPlayerText = std::make_shared<DynamicTextElement>(textGetter, (float)12, textColor, true);
			this->add_child(this->currentPlayerText);

			this->set_screen_area(this->screenArea);
		}

		void set_screen_area(sdl::FRect& screenArea) override {
			UIElement::set_screen_area(screenArea);

			if (this->miniMap) {
				sdl::FRect mapArea(this->padding, this->padding, this->screenArea.w - 2 * this->padding, this->screenArea.w - 2 * this->padding);
				sdl::FPoint tileSize{ (float)this->screenArea.w / this->state->tiles().width() , (float)this->screenArea.h / this->state->tiles().height() };
				this->miniMap->set_screen_area(mapArea);
				this->miniMap->set_tile_size(std::min(tileSize.x, tileSize.y));

				sdl::FRect currentPlayerTextArea{ 0, mapArea.y + mapArea.h + this->padding, this->screenArea.w, 10 };
				this->currentPlayerText->set_screen_area(currentPlayerTextArea);
			}
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
				const int sideBarWidth = 200;

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