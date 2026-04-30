export module GameController;

import WorldState;
import std;
import Events;
import WorldGenerator;

namespace OpenKaiser {
	export class GameController {
	private:
		std::shared_ptr<WorldState> state_;
		Event<std::uint16_t> on_start_human_turn_;
		std::random_device rd_;
		std::mt19937 random_;

	public:
		GameController()
			: random_(rd_()) {
		}

		Event<std::uint16_t>& on_start_human_turn() {
			return on_start_human_turn_;
		}

		std::shared_ptr<WorldState>& StartGame() {
			state_ = WorldGenerator().Generate(WorldConfig(), random_);
			state_->Save("save/init.txt");
			return state_;
		}

		void EndRound() {
			// Reset population for countries
			for (Country& country : state_->countries()) {
				country.population = 0;
			}

			// Feed the population
			std::uniform_real_distribution<float> growth_dist(1.03, 1.07);
			for (int x = 0; x < state_->tiles().width(); x++) {
				for (int y = 0; y < state_->tiles().height(); y++) {
					Tile& tile = state_->tiles()(x, y);

					// Feed the population
					if (tile.population > 0) {
						int neededFood = tile.population;
						Country& country = state_->countries()[tile.countryId];

						// Consume livestock first, then wheat
						int consumed = std::min(neededFood, country.livestock);
						neededFood -= consumed;
						country.livestock -= consumed;

						consumed = std::min(neededFood, state_->countries()[tile.countryId].wheat);
						neededFood -= consumed;
						country.wheat -= consumed;

						// Can only grow if everyone was fed
						if (neededFood == 0) {
							tile.population *= growth_dist(random_);
						}

						country.population += tile.population;
					}
				}
			}

			this->state_->next_year();

			// Income
			std::uniform_int_distribution livestock_dist(40, 60);
			std::uniform_int_distribution wheat_dist(40, 60);
			std::uniform_real_distribution<float> gold_dist(0.63f, 0.87f);

			for (int x = 0; x < state_->tiles().width(); x++) {
				for (int y = 0; y < state_->tiles().height(); y++) {
					Tile& tile = state_->tiles()(x, y);
					if (tile.countryId >= 0) {
						Country& country = state_->countries()[tile.countryId];
						switch (tile.building) {
						case BuildingType::Pasture:
							country.livestock += livestock_dist(random_);
							break;
						case BuildingType::Field:
							country.wheat += wheat_dist(random_);
							break;
						case BuildingType::Village:
						case BuildingType::Market:
						case BuildingType::Town:
							country.gold += tile.population * gold_dist(random_);
							break;
						}
					}
				}
			}
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
