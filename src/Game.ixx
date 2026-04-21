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

		int width = 1280;
		int height = 800;

		WorldState world;

		bool Update() {
			sdl::Event event;
			while (sdl::poll_event(event)) {
				if (event.type == sdl::EventType::Quit) {
					std::cout << "Quitting...\n";
					return false;
				}
				if (event.type == sdl::EventType::KeyDown) {
					if(event.key.key == 32) {
						std::cout << "Regenerating World...\n";
						InitWorldState();
					}
				}
			}
			return true;
		}

		void Draw() {
			sdl::set_render_draw_color(this->renderer, 11, 11, 11, 255);
			sdl::render_clear(this->renderer);

			const Uint8 tileSize = 16;
			for (int x = 0; x < (this->width / tileSize); x++) {
				for (int y = 0; y < (this->height / tileSize); y++) {
					if (x >= this->world.tiles().extent(0) || y >= this->world.tiles().extent(1)) {
						continue;
					}

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

					sdl::FRect rect = { x * (float)tileSize, y * (float)tileSize, tileSize, tileSize};
					sdl::render_fill_rect(this->renderer, rect);

					// Draw country borders
					if (currentTile.countryId != 0) {
						// Left
						if (x > 0 && currentTile.countryId != this->world.tiles()[x - 1, y].countryId) {
							sdl::set_render_draw_color(this->renderer, 0, 0, 0, 255);
							sdl::render_line(this->renderer, x * tileSize, y * tileSize, x * tileSize, (y + 1) * tileSize);
						}
						// Top
						if (y > 0 && currentTile.countryId != this->world.tiles()[x, y - 1].countryId) {
							sdl::set_render_draw_color(this->renderer, 0, 0, 0, 255);
							sdl::render_line(this->renderer, x * tileSize, y * tileSize, (x + 1) * tileSize, y * tileSize);
						}
						// Right
						if (x < this->world.tiles().extent(0) - 1 && currentTile.countryId != this->world.tiles()[x - 1, y].countryId) {
							sdl::set_render_draw_color(this->renderer, 0, 0, 0, 255);
							sdl::render_line(this->renderer, (x + 1) * tileSize - 1, y * tileSize, (x + 1) * tileSize - 1, (y + 1) * tileSize);
						}
						// Bottom
						if (y < this->world.tiles().extent(1) - 1 && currentTile.countryId != this->world.tiles()[x, y - 1].countryId) {
							sdl::set_render_draw_color(this->renderer, 0, 0, 0, 255);
							sdl::render_line(this->renderer, x * tileSize, (y + 1) * tileSize - 1, (x + 1) * tileSize, (y + 1) * tileSize - 1);
						}
					}
				}
			}

			sdl::render_present(this->renderer);
		}

		void InitWorldState() {
			std::cout << "Initializing World...\n";
			this->world = WorldState(80, 50);
			FloatMapGenerator().generate(this->world);
			this->world.tiles()[10, 10].countryId = 1;
		}

	public:
		void Init() {
			InitWorldState();

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
