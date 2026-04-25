export module General;

import std;
import SDL3;
import ResourceManager;
import WorldState;

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
		std::string id;

	public:
		virtual void set_screen_area(sdl::FRect& screenArea) {
			this->screenArea = screenArea;
		}

		sdl::FRect& get_screen_area() {
			return this->screenArea;
		}

		void set_id(const std::string& id) {
			this->id = id;
		}

		std::string& get_id() {
			return this->id;
		}

		virtual void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources) {
			this->renderer = renderer;
			this->resourceManager = resources;
		}

		virtual void update(float passedTime) {
			for (auto element : children) {
				element->update(passedTime);
			}
		}

		virtual void draw(WorldState& state) {
			for (auto element : children) {
				element->draw(state);
			}
		}
		virtual void handle_event(WorldState& state, sdl::Event& event) {
			for (auto element : children) {
				element->handle_event(state, event);
			}
		};
	};
}