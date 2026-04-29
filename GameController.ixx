export module GameController;

import WorldState;
import std;
import Events;

namespace OpenKaiser {

	export class Command {
	public:
		virtual void Execute(std::shared_ptr<WorldState>& world) = 0;
	};

	export class EndTurnCommand : public Command {
	public:
		void Execute(std::shared_ptr<WorldState>& world) {
			if (world->next_player() == 0) {
				world->next_year();
			}
		}
	};

	export class GameController {
	private:
		std::shared_ptr<WorldState> state_;
		std::vector<std::unique_ptr<Command>> history_;
		Event<Command*> before_command;
		Event<Command*> after_command;

	public:
		void Init(std::shared_ptr<WorldState>& state) {
			state_ = state;
		}

		void HandleCommand(std::unique_ptr<Command> command) {
			this->before_command.emit(command.get());
			command->Execute(state_);
			history_.push_back(std::move(command));
			this->after_command.emit(command.get());
		}

		int history_size() const {
			return history_.size();
		}

		Event<Command*>& on_before_command() {
			return this->before_command;
		}

		Event<Command*>& on_after_command() {
			return this->after_command;
		}
	};

}
