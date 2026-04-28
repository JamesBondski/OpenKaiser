import Game;
import std;

int main()
{
	try {
		auto game = OpenKaiser::Game();
		game.Init();
		game.Run();
	}
	catch (const std::runtime_error& e) {
		std::cout << typeid(e).name() << e.what() << std::endl;
	}
}
