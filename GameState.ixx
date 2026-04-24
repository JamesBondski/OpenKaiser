export module GameState;

import std;
import SDL3;
import WorldState;
import ResourceManager;
import MapRenderer;

namespace OpenKaiser {

	export class GameState {
	protected:
		sdl::RendererPtr renderer;
		std::shared_ptr<ResourceManager> resourceManager;
	public:
		virtual void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager> resources) {
			this->renderer = renderer;
			this->resourceManager = resources;
		}
		virtual void update(float passedTime) = 0;
		virtual void draw(WorldState state) = 0;
		virtual void handle_event(sdl::Event& event) = 0;
	};

	export class MainState : public GameState {
	private:
		MapRenderer mapRenderer;
		int width;
		int height;

	public:
		void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager> resources)  override {
			GameState::init(renderer, resources);
			sdl::get_current_render_output_size(renderer, &this->width, &this->height);

			sdl::FRect mapArea(0, 0, this->width, this->height);
			this->mapRenderer.init(this->resourceManager, this->renderer, mapArea);
			this->mapRenderer.set_render_mode(RenderMode::Image);
		}

		void update(float passedTime) override {

		}

		void draw(WorldState state) override {
			this->mapRenderer.draw(state);
		}

		void handle_event(sdl::Event& event) override {

		}
	};
}
