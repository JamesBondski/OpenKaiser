import Game;
import std;

int main()
{
	try {
		auto game = OpenKaiser::Game();
		game.init();
		game.run();
	}
	catch (const std::runtime_error& e) {
		std::cout << typeid(e).name() << e.what() << std::endl;
	}
}
