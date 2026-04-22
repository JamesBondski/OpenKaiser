export module Game;
import std;
import SDL3;
import WorldState;
import MapGenerator;
import General;

using std::uint8_t;

namespace OpenKaiser {

	export class Game {
	private:
		sdl::WindowPtr window;
		sdl::RendererPtr renderer;

		int width = 1280;
		int height = 800;

		WorldState world;

		std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> country_colors;

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
						this->init_world_state();
					}
				}
			}
			return true;
		}

		void load_country_colors() {
			const std::string config_location = "data/config/country_colors.txt";
			std::ifstream config_file(config_location);
			// Check for errors
			if (config_file.bad()) {
				throw OpenKaiserError("Could not open Country Colors file in " + config_location);
			}

			std::string line;
			while (std::getline(config_file, line)) {
				std::stringstream splitter(line);
				int r, g, b;
				splitter >> r >> g >> b;

				if (!splitter.good()) {
					throw OpenKaiserError("Error parsing color: " + line);
				}

				country_colors.push_back({
					static_cast<uint8_t>(r),
					static_cast<uint8_t>(g),
					static_cast<uint8_t>(b)
					});
			}
		}

		void draw() {
			sdl::set_render_draw_color(this->renderer, 11, 11, 11, 255);
			sdl::render_clear(this->renderer);

			const float tileSize = 16;
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

					sdl::FRect rect = { x * tileSize, y * tileSize, tileSize, tileSize};
					sdl::render_fill_rect(this->renderer, rect);

					// Draw country borders
					if (currentTile.countryId >= 0) {
						auto cc = this->country_colors[currentTile.countryId];
						uint8_t r = std::get<0>(cc);
						sdl::set_render_draw_color(this->renderer, std::get<0>(cc), std::get<0>(cc), std::get<0>(cc), 255);
						// Left
						if (x > 0 && currentTile.countryId != this->world.tiles()[x - 1, y].countryId) {
							sdl::render_line(this->renderer, x * tileSize, y * tileSize, x * tileSize, (y + 1) * tileSize);
						}
						// Top
						if (y > 0 && currentTile.countryId != this->world.tiles()[x, y - 1].countryId) {
							sdl::render_line(this->renderer, x * tileSize, y * tileSize, (x + 1) * tileSize, y * tileSize);
						}
						// Right
						if (x < this->world.tiles().extent(0) - 1 && currentTile.countryId != this->world.tiles()[x + 1, y].countryId) {
							sdl::render_line(this->renderer, (x + 1) * tileSize - 1, y * tileSize, (x + 1) * tileSize - 1, (y + 1) * tileSize);
						}
						// Bottom
						if (y < this->world.tiles().extent(1) - 1 && currentTile.countryId != this->world.tiles()[x, y + 1].countryId) {
							sdl::render_line(this->renderer, x * tileSize, (y + 1) * tileSize - 1, (x + 1) * tileSize, (y + 1) * tileSize - 1);
						}
					}
				}
			}

			sdl::render_present(this->renderer);
		}

		void init_world_state() {
			std::cout << "Initializing World...\n";
			this->world = WorldState(80, 50);
			FloatMapGenerator().generate(this->world);
			this->world.tiles()[10, 10].countryId = 2;
			this->world.tiles()[11, 10].countryId = 2;
		}

	public:
		void init() {
			init_world_state();
			load_country_colors();

			std::cout << "Initializing SDL...\n";
			sdl::init();
			this->window = sdl::create_window("OpenKaiser", this->width, this->height, 0);
			this->renderer = sdl::create_renderer(this->window.get());
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
