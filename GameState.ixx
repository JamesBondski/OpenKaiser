export module GameState;

import std;
import SDL3;
import WorldState;
import ResourceManager;
import MapRenderer;
import Menu;
import General;

namespace OpenKaiser {
	export class GameState : public UIElement {
	protected:
		std::string nextState;

		void fill_screen() {

		}
	public:

		std::string& get_next_state() {
			return nextState;
		}

		void set_next_state(const std::string& stateName) {
			nextState = stateName;
		}

		void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			UIElement::init(renderer, resources, state, controller);

			// Set screen area to whole screen
			int width, height;
			sdl::get_current_render_output_size(renderer, &width, &height);
			sdl::FRect ownArea{ 0, 0, width, height };
			this->set_screen_area(ownArea);
		}

		void handle_event(sdl::Event& event) override {
			if (event.type == sdl::EventType::WindowResized) {
				sdl::FRect ownArea{ 0, 0, event.window.data1, event.window.data2};
				this->set_screen_area(ownArea);
			}
			UIElement::handle_event(event);
		}
	};

	export class MainState : public GameState {
	private:
		std::shared_ptr<MapRenderer> mapRenderer;
		std::shared_ptr<MenuManager> menu;

	public:
		void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			GameState::init(renderer, resources, state, controller);

			this->mapRenderer = std::make_shared<MapRenderer>();
			this->mapRenderer->init(this->resourceManager, this->renderer, this->state, this->controller);
			this->mapRenderer->set_render_mode(RenderMode::Image);
			this->children.push_back(this->mapRenderer);

			this->menu = std::make_shared<MenuManager>();
			this->menu->init(renderer, resources, state, controller);
			
			this->children.push_back(this->menu);

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
				sdl::FRect mapArea(0, 0, this->screenArea.w, this->screenArea.h - menuHeight);
				this->mapRenderer->set_screen_area(mapArea);

				sdl::FRect menuArea{ 0, mapArea.y + mapArea.h, this->screenArea.w, menuHeight };
				this->menu->set_screen_area(menuArea);
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
