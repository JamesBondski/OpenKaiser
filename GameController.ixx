export module GameController;

import WorldState;
import std;
import Events;

namespace OpenKaiser {
	export class GameController {
	private:
		std::shared_ptr<WorldState> state_;
		Event<std::uint16_t> on_start_human_turn_;

	public:
		Event<std::uint16_t>& on_start_human_turn() {
			return on_start_human_turn_;
		}

		void Init(std::shared_ptr<WorldState>& state) {
			state_ = state;
		}

		void EndRound() {
			this->state_->next_year();
		}

		void EndTurn() {
			std::uint16_t next_country = state_->next_player();
			if (next_country == 0) {
				EndRound();
			}

			Dynasty& dynasty = state_->current_dynasty();
			if (dynasty.human) {
				on_start_human_turn_.emit(next_country);
			}
			else {
				// AI turn here
				EndTurn();
			}
		}
	};
}
