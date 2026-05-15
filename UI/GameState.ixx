export module UI:GameState;

import std;
import SDL3;
import :Stacks;

namespace OpenKaiser {
	export class GameState : public HorizontalStack {
	private:
		void UpdateScreenArea() {
			int width, height;
			sdl::get_current_render_output_size(this->renderer_, &width, &height);

			sdl::FRect own_area{ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height) };
			HorizontalStack::set_screen_area(own_area);
		}
	protected:
		std::string next_state_;

	public:

		std::string& next_state() {
			return next_state_;
		}

		void set_next_state(const std::string& state_name) {
			next_state_ = state_name;
		}

		void Init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			HorizontalStack::Init(renderer, resources, state, controller);
			UpdateScreenArea();
		}

		void HandleEvent(sdl::Event& event) override {
			if (event.type == sdl::EventType::WindowResized) {
				UpdateScreenArea();
			}
			HorizontalStack::HandleEvent(event);
		}
	};
}
