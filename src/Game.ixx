export module Game;
import std;
import SDL3;

namespace OpenKaiser {
	export class Game {
	private:
		sdl::WindowPtr window;
		sdl::RendererPtr renderer;

		bool Update() {
			sdl::Event event;
			while (sdl::poll_event(event)) {
				if (event.type == sdl::EventType::Quit) {
					std::cout << "Quitting...\n";
					return false;
				}
			}
			return true;
		}

		void Draw() {
			sdl::set_render_draw_color(this->renderer, 11, 11, 11, 255);
			sdl::render_clear(this->renderer);

			sdl::set_render_draw_color(this->renderer, 255, 0, 0, 255);
			sdl::render_debug_text(this->renderer, 10, 10, "Hello World!");

			sdl::render_present(this->renderer);
		}

	public:
		void Init() {
			std::cout << "Initializing...\n";
			sdl::init();
			this->window = sdl::create_window("OpenKaiser", 800, 600, 0);
			this->renderer = sdl::create_renderer(this->window.get());
		}

		void Run() {
			std::cout << "Running...\n";
			try {
				while (this->Update()) {
					this->Draw();
				}
			}
			catch (const sdl::sdl_error& e) {
				throw;
			}
		}
	};
}
