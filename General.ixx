export module General;

import std;
import SDL3;
import ResourceManager;
import WorldState;
import GameController;

namespace OpenKaiser {

    export class OpenKaiserError : public std::runtime_error {
    public:
        OpenKaiserError(const std::string& msg)
            : std::runtime_error(msg) {}
    };

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

		virtual void draw() {
			for (auto element : children) {
				sdl::Rect clipRect{ this->screenArea.x + element->get_screen_area().x, this->screenArea.y + element->get_screen_area().y, element->get_screen_area().w, element->get_screen_area().h };
				sdl::set_render_clip_rect(this->renderer, &clipRect);
				element->draw();
				sdl::set_render_clip_rect(this->renderer, nullptr);
			}
		}
		virtual void handle_event(sdl::Event& event) {
			for (auto element : children) {
				element->handle_event(event);
			}
		};
	};
}