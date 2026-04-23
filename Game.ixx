export module Game;
import std;
import SDL3;
import WorldState;
import WorldGenerator;
import General;
import MapRenderer;

using std::uint8_t;

namespace OpenKaiser {

	export class Game {
	private:
		sdl::WindowPtr window;
		sdl::RendererPtr renderer;

		MapRenderer mapRenderer;

		int width = 1280;
		int height = 800;

		WorldState world;

		bool update() {
			sdl::Event event;
			while (sdl::poll_event(event)) {
				if (event.type == sdl::EventType::Quit) {
					std::cout << "Quitting...\n";
					return false;
				}
				if (event.type == sdl::EventType::KeyDown) {
					if(event.key.key == 32) {
						std::cout << "Regenerating World...\n";
						this->world = WorldGenerator().generate(WorldConfig());
					}
				}
			}
			return true;
		}

		void draw() {
			sdl::set_render_draw_color(this->renderer, 11, 11, 11, 255);
			sdl::render_clear(this->renderer);

			this->mapRenderer.draw(this->renderer, this->world);
		}

	public:
		void init() {
			std::cout << "Initializing World...\n";
			this->world = WorldGenerator().generate(WorldConfig());

			std::cout << "Initializing SDL...\n";
			sdl::init();
			this->window = sdl::create_window("OpenKaiser", this->width, this->height, 0);
			this->renderer = sdl::create_renderer(this->window.get());

			std::cout << "Initializung UI...\n";
			sdl::FRect mapArea(0, 0, this->width, this->height);
			this->mapRenderer.set_screenArea(mapArea);
		}

		void run() {
			std::cout << "Running...\n";
			try {
				while (this->update()) {
					this->draw();
				}
			}
			catch (const sdl::sdl_error& e) {
				throw;
			}
		}
	};
}
