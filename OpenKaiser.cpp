import Game;
import std;

int main()
{
	try {
		auto game = OpenKaiser::Game();
		game.init();
		game.run();
	}
	catch (const sdl::sdl_error& e) {
		std::cout << "SDL Error: " << e.what() << std::endl;
	}
}
