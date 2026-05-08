export module Game;
import std;
import SDL3;
import WorldState;
import WorldGenerator;
import General;
import MapRenderer;
import ResourceManager;
import MainState;
import GameController;
import UI;

using std::uint8_t;

namespace OpenKaiser {

	export class Game {
	private:
		std::shared_ptr<GameController> controller_;
		std::string current_state_;
		std::unordered_map<std::string, std::unique_ptr<GameState>> states_;
		std::shared_ptr<ResourceManager> resource_manager_;
		MapRenderer map_renderer_;

		sdl::WindowPtr window_;
		sdl::RendererPtr renderer_;

		int width_ = 1280;
		int height_ = 800;

		std::shared_ptr<WorldState> world_;
		std::uint64_t last_update_ = 0;

		std::unique_ptr<GameState>& State() {
			return states_[current_state_];
		}

		bool Update() {
			if (last_update_ = 0) {
				last_update_ = sdl::get_performance_counter();
			}
			std::uint64_t now = sdl::get_performance_counter();
			float diff = (last_update_ - now) / (float)sdl::get_performance_frequency() * 1000;
			last_update_ = now;

			sdl::Event event;
			while (sdl::poll_event(event)) {
				if (event.type == sdl::EventType::Quit) {
					std::cout << "Quitting...\n";
					return false;
				}
				State()->HandleEvent(event);
			}

			State()->Update(diff);

			std::string next_state = State()->next_state();
			if (next_state == "quit") {
				return false;
			}

			if (!next_state.empty()) {
				State()->set_next_state("");
				current_state_ = State()->next_state();
			}

			return true;
		}

		void Draw() {
			sdl::set_render_draw_color(renderer_, { 11, 11, 11, 255 });
			sdl::render_clear(renderer_);

			State()->Draw();

			sdl::render_present(renderer_);
		}

		template <typename T> 
			requires std::derived_from<T, GameState> && std::is_default_constructible_v<T>
		void AddGameState(const std::string& name) {
			T* new_state = new T();
			states_.insert(std::pair<std::string, std::unique_ptr<GameState>>(name, std::unique_ptr<GameState>(new_state)));
			new_state->Init(renderer_, resource_manager_, world_, controller_);
		}

	public:
		void Init() {
			std::cout << "Initializing World...\n";

			controller_ = std::make_shared<GameController>();
			world_ = controller_->StartGame();

			std::cout << "Initializing SDL...\n";
			sdl::init();
			window_ = sdl::create_window("OpenKaiser", width_, height_, 0x20);
			renderer_ = sdl::create_renderer(window_.get());
			sdl::set_render_vsync(renderer_, 1);

			sdl::ttf_init();

			std::cout << "Initializung UI...\n";
			resource_manager_.reset(new ResourceManager());
			resource_manager_->Init(renderer_);

			AddGameState<MainState>("main");
			current_state_ = "main";
		}

		void Run() {
			std::cout << "Running...\n";
			try {
				while (Update()) {
					Draw();
				}
			}
			catch (const sdl::sdl_error&) {
				world_->Save("save/crash.txt");
				throw;
			}
		}
	};
}
