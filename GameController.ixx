export module GameController;

import WorldState;
import std;

namespace OpenKaiser {

	export class Command {
	public:
		virtual void Execute(std::shared_ptr<WorldState>& world) = 0;
	};

	export class EndTurnCommand : public Command {
	public:
		void Execute(std::shared_ptr<WorldState>& world) {
			if (world->nextPlayer() == 0) {
				// End Round
			}
		}
	};

	export class GameController {
	private:
		std::shared_ptr<WorldState> state;
		std::vector<std::unique_ptr<Command>> history;

	public:
		void init(std::shared_ptr<WorldState>& state) {
			this->state = state;
		}

		void HandleCommand(std::unique_ptr<Command> command) {
			command->Execute(this->state);
			history.push_back(std::move(command));
		}
	};

}
