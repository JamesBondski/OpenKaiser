import Game;
import std;
import Logging;

int main()
{
	try {
		OpenKaiser::Logger::InitBoth("openkaiser.log", OpenKaiser::LogSeverity::Debug);
		auto game = OpenKaiser::Game();
		game.Init();
		game.Run();
	}
	catch (const std::runtime_error& e) {
		std::cout << typeid(e).name() << e.what() << std::endl;
	}
}
