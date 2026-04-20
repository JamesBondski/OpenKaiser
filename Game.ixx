export module Game;
import <iostream>;
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
			
		}

	public:
		void Init() {
			std::cout << "Initializing...\n";
			sdl::init();
			this->window = sdl::create_window("OpenKaiser", 800, 600, 0);
			this->renderer = sdl::create_renderer(this->window.get(), "OpenKaiser");
		}

		void Run() {
			std::cout << "Running...\n";
			while (this->Update()) {
				this->Draw();
			}
		}
	};
}
