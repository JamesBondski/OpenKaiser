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
	public:

		std::string& get_next_state() {
			return nextState;
		}

		void set_next_state(const std::string& stateName) {
			nextState = stateName;
		}
	};

	export class MainState : public GameState {
	private:
		std::shared_ptr<MapRenderer> mapRenderer;
		int width;
		int height;
		std::shared_ptr<MenuManager> menu;

	public:
		void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state)  override {
			UIElement::init(renderer, resources, state);
			sdl::get_current_render_output_size(renderer, &this->width, &this->height);

			this->mapRenderer = std::make_shared<MapRenderer>();
			sdl::FRect mapArea(0, 0, this->width, this->height - 200);
			this->mapRenderer->init(this->resourceManager, this->renderer, this->state);
			this->mapRenderer->set_screen_area(mapArea);
			this->mapRenderer->set_render_mode(RenderMode::Image);
			this->children.push_back(this->mapRenderer);

			this->menu = std::make_shared<MenuManager>();
			this->menu->init(renderer, resources, state);
			sdl::FRect menuArea{ 0, this->height - 200, this->width, 200 };
			this->menu->set_screen_area(menuArea);
			this->children.push_back(this->menu);

			auto rootItem = this->menu->getRootItem();
			std::shared_ptr<MenuItem> quit = std::make_shared<MenuItem>();
			quit->name = "quit";
			quit->text = "(Q)uit";
			quit->hotkey = 0x00000070u; // Q
			quit->callback = [this](const std::string itemName) { return this->handle_quit(itemName); };
			this->menu->add_item(rootItem, quit);
		}

		void handle_event(sdl::Event& event) override {
			if (event.type == sdl::EventType::KeyDown) {
				if (event.key.key == 112) {
					std::cout << "Saving map.." << std::endl;
					sdl::SurfacePtr mapSurface = this->mapRenderer->render_to_surface();
					sdl::save_png(mapSurface, "map.png");
				}
			}
			GameState::handle_event(event);
		}

		ResultAction handle_quit(const std::string itemName) {
			this->set_next_state("quit");
			return ResultAction::None;
		}
	};
}
