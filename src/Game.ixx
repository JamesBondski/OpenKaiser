export module Game;
import std;
import SDL3;
import WorldState;
import MapGenerator;

namespace OpenKaiser {
	export class Game {
	private:
		sdl::WindowPtr window;
		sdl::RendererPtr renderer;

		int width = 800;
		int height = 600;

		WorldState world;

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

			for (int x = 0; x < (this->width / 64); x++) {
				for (int y = 0; y < (this->height / 64); y++) {
					Tile currentTile = this->world.tiles()[x,y];

					switch (currentTile.type) {
					case TileType::Grass:
						sdl::set_render_draw_color(this->renderer, 76, 153, 0, 255);
						break;
					case TileType::Water:
						sdl::set_render_draw_color(this->renderer, 41, 128, 185, 255);
						break;
					case TileType::Mountain:
						sdl::set_render_draw_color(this->renderer, 127, 140, 141, 255);
						break;
					}

					sdl::FRect rect = { x * 64.0f, y * 64.0f, 63.0f, 63.0f };
					sdl::render_fill_rect(this->renderer, rect);
				}
			}

			sdl::render_present(this->renderer);
		}

	public:
		void Init() {
			std::cout << "Initializing World...\n";
			this->world = WorldState(100, 100);
			FloatMapGenerator().generate(this->world);

			std::cout << "Initializing SDL...\n";
			sdl::init();
			this->window = sdl::create_window("OpenKaiser", this->width, this->height, 0);
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
