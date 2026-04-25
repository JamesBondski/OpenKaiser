export module Game;
import std;
import SDL3;
import WorldState;
import WorldGenerator;
import General;
import MapRenderer;
import ResourceManager;
import GameState;

using std::uint8_t;

namespace OpenKaiser {

	export class Game {
	private:
		sdl::WindowPtr window;
		sdl::RendererPtr renderer;

		MapRenderer mapRenderer;
		std::shared_ptr<ResourceManager> resourceManager;
		std::unordered_map<std::string, std::unique_ptr<GameState>> states;
		std::string currentState;

		int width = 1280;
		int height = 800;

		WorldState world;
		std::uint64_t lastUpdate = 0;

		std::unique_ptr<GameState>& state() {
			return states[this->currentState];
		}

		bool update() {
			if (lastUpdate = 0) {
				lastUpdate = sdl::get_performance_counter();
			}
			std::uint64_t now = sdl::get_performance_counter();
			float diff = (lastUpdate - now) / (float)sdl::get_performance_frequency() * 1000;
			lastUpdate = now;

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
				this->state()->handle_event(this->world, event);
			}

			this->state()->update(diff);

			std::string nextState = this->state()->get_next_state();
			if (nextState == "quit") {
				return false;
			}

			if (!nextState.empty()) {
				this->state()->set_next_state("");
				this->currentState = this->state()->get_next_state();
			}

			return true;
		}

		void draw() {
			sdl::set_render_draw_color(this->renderer, { 11, 11, 11, 255 });
			sdl::render_clear(this->renderer);

			this->state()->draw(this->world);

			sdl::render_present(renderer);
		}

		template <typename T> 
			requires std::derived_from<T, GameState> && std::is_default_constructible_v<T>
		void add_gamestate(const std::string& name) {
			T* newState = new T();
			this->states.insert(std::pair<std::string, std::unique_ptr<GameState>>(name, std::unique_ptr<GameState>(newState)));
			newState->init(this->renderer, this->resourceManager);
		}

	public:
		void init() {
			std::cout << "Initializing World...\n";
			this->world = WorldGenerator().generate(WorldConfig());
			this->world.save("save/init.txt");

			std::cout << "Initializing SDL...\n";
			sdl::init();
			this->window = sdl::create_window("OpenKaiser", this->width, this->height, 0);
			this->renderer = sdl::create_renderer(this->window.get());

			sdl::ttf_init();

			std::cout << "Initializung UI...\n";
			this->resourceManager.reset(new ResourceManager());
			this->resourceManager->init(this->renderer);

			// Initialize GameStates
			this->add_gamestate<MainState>("main");
			this->currentState = "main";
		}

		void run() {
			std::cout << "Running...\n";
			try {
				while (this->update()) {
					this->draw();
				}
			}
			catch (const sdl::sdl_error& e) {
				this->world.save("save/crash.txt");
				throw;
			}
		}
	};
}
