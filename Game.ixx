export module Game;
import std;
import SDL3;
import WorldState;
import WorldGenerator;
import General;
import MapRenderer;
import ResourceManager;
import GameState;
import GameController;

using std::uint8_t;

namespace OpenKaiser {

	export class Game {
	private:
		std::shared_ptr<GameController> controller;
		std::string currentState;
		std::unordered_map<std::string, std::unique_ptr<GameState>> states;
		std::shared_ptr<ResourceManager> resourceManager;
		MapRenderer mapRenderer;

		sdl::WindowPtr window;
		sdl::RendererPtr renderer;

		int width = 1280;
		int height = 800;

		std::shared_ptr<WorldState> world;
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
				this->state()->handle_event(event);
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

			this->state()->draw();

			sdl::render_present(renderer);
		}

		template <typename T> 
			requires std::derived_from<T, GameState> && std::is_default_constructible_v<T>
		void add_gamestate(const std::string& name) {
			T* newState = new T();
			this->states.insert(std::pair<std::string, std::unique_ptr<GameState>>(name, std::unique_ptr<GameState>(newState)));
			newState->init(this->renderer, this->resourceManager, this->world, this->controller);
		}

	public:
		void init() {
			std::cout << "Initializing World...\n";
			this->world = WorldGenerator().generate(WorldConfig());
			this->world->save("save/init.txt");

			this->controller = std::make_shared<GameController>();
			this->controller->init(this->world);

			std::cout << "Initializing SDL...\n";
			sdl::init();
			this->window = sdl::create_window("OpenKaiser", this->width, this->height, 0x20); // 0x20=Resizable
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
				this->world->save("save/crash.txt");
				throw;
			}
		}
	};
}
