export module UI;

import std;
import SDL3;
import WorldState;
import ResourceManager;
import General;
import GameController;

namespace OpenKaiser {
	export class UIElement {
	protected:
		sdl::FRect screenArea;
		sdl::RendererPtr renderer;
		std::shared_ptr<ResourceManager> resourceManager;
		std::vector<std::shared_ptr<UIElement>> children;
		std::shared_ptr<WorldState> state;
		std::string id;
		std::shared_ptr<GameController> controller;

	public:
		virtual void set_screen_area(sdl::FRect& screenArea) {
			this->screenArea = screenArea;
		}

		sdl::FRect& get_screen_area() {
			return this->screenArea;
		}

		void set_world_state(std::shared_ptr<WorldState>& state) {
			this->state = state;
			for (auto element : children) {
				element->set_world_state(state);
			}
		}

		void set_id(const std::string& id) {
			this->id = id;
		}

		std::string& get_id() {
			return this->id;
		}

		virtual void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller) {
			this->renderer = renderer;
			this->resourceManager = resources;
			this->state = state;
			this->controller = controller;
		}

		virtual void update(float passedTime) {
			for (auto element : children) {
				element->update(passedTime);
			}
		}

		virtual void draw(sdl::Point& offset) {
			sdl::Point newOffset{ offset.x + this->screenArea.x, offset.y + this->screenArea.y };
			for (auto element : children) {
				sdl::Rect clipRect{ newOffset.x + element->get_screen_area().x, newOffset.y + element->get_screen_area().y, element->get_screen_area().w, element->get_screen_area().h };
				sdl::set_render_clip_rect(this->renderer, &clipRect);
				element->draw(newOffset);
				sdl::set_render_clip_rect(this->renderer, nullptr);
			}
		}
		virtual void handle_event(sdl::Event& event) {
			for (auto element : children) {
				element->handle_event(event);
			}
		};
	};

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
				sdl::FRect ownArea{ 0, 0, event.window.data1, event.window.data2 };
				this->set_screen_area(ownArea);
			}
			UIElement::handle_event(event);
		}
	};
}
