export module GameController;

import WorldState;
import std;
import Events;
import WorldGenerator;

namespace OpenKaiser {

	export class Command {
	protected:
		std::vector<std::unique_ptr<Command>> sub_commands_;

		template <std::derived_from<Command> T, typename... Args>
		void Enqueue(Args&&... args) {
			sub_commands_.push_back(std::make_unique<T>(std::forward<Args>(args)...));
		}
	public:
		virtual void Execute(std::shared_ptr<WorldState>& state, std::mt19937& random) = 0;
		
		std::vector<std::unique_ptr<Command>> DoExecute(std::shared_ptr<WorldState>& state, std::mt19937 random) {
			Execute(state, random);
			return std::move(sub_commands_);
		}
	};

	export class ChangeResourceAmountCommand : public Command {
	private:
		std::uint16_t country_id_;
		ResourceType resource_;
		int amount_;

	public:
		ChangeResourceAmountCommand(std::uint16_t country_id, ResourceType resource, int amount) 
			: country_id_(country_id), resource_(resource), amount_(amount) {
		}

		void Execute(std::shared_ptr<WorldState>& state, std::mt19937& random) override {
			state->country(country_id_).resources[resource_] += amount_;
		}
	};

	export class FeedPopulationCommand : public Command {
	private:
		int FeedPopulation(Country& country, ResourceType resource, int neededFood) {
			int consumed = std::min(neededFood, country.resources[resource]);
			if (consumed > 0) {
				country.population_fed += consumed;
				Enqueue<ChangeResourceAmountCommand>(country.id, ResourceType::Livestock, -consumed);
			}
			return consumed;
		}
	public:
		void Execute(std::shared_ptr<WorldState>& state, std::mt19937& random) override {
			for (int x = 0; x < state->tiles().width(); x++) {
				for (int y = 0; y < state->tiles().height(); y++) {
					Tile& tile = state->tile(x, y);

					// Feed the population
					if (tile.population > 0) {
						int neededFood = tile.population;
						Country& country = state->country(tile.countryId);
						country.population += tile.population;

						// Consume livestock first, then wheat
						neededFood -= FeedPopulation(country, ResourceType::Livestock, neededFood);
						neededFood -= FeedPopulation(country, ResourceType::Wheat, neededFood);
					}
				}
			}
		}
	};

	export class ResetPopulationCountCommand : public Command {
	public:
		void Execute(std::shared_ptr<WorldState>& state, std::mt19937& random) override {
			std::vector<bool> country_fed;
			for (Country& country : state->countries()) {
				country.population = 0;
				country_fed.push_back(true);
			}
		}
	};

	export class ChangePopulationCountCommand : public Command {
	private:
		int amount_, x_, y_;
	public:
		ChangePopulationCountCommand(int x, int y, int amount) : amount_(amount), x_(x), y_(y) {
		}

		void Execute(std::shared_ptr<WorldState>& state, std::mt19937& random) override {
			Tile& tile = state->tile(x_, y_);
			Country& country = state->country(tile.countryId);

			tile.population += amount_;
			country.population += amount_;
		}
	};

	export class GrowPopulationCommand : public Command {
	private:
		int x_, y_;
	public:
		GrowPopulationCommand(int x, int y) 
			: x_(x), y_(y){
		}

		void Execute(std::shared_ptr<WorldState>& state, std::mt19937& random) override {
			Tile& tile = state->tile(x_, y_);
			Country& country = state->country(tile.countryId);
			if (country.population_fed >= country.population) {
				std::uniform_real_distribution<float> growth_dist(1.03f, 1.07f);
				int growth_amount = static_cast<int>(tile.population * growth_dist(random)) - tile.population;
				Enqueue<ChangePopulationCountCommand>(x_, y_, growth_amount);
			}
		}
	};

	export class IncomeCommand : public Command {
	public:
		void Execute(std::shared_ptr<WorldState>& state, std::mt19937& random) override {
			// Income
			std::uniform_int_distribution livestock_dist(40, 60);
			std::uniform_int_distribution wheat_dist(40, 60);
			std::uniform_real_distribution<float> gold_dist(0.63f, 0.87f);
			
			for (int x = 0; x < state->tiles().width(); x++) {
				for (int y = 0; y < state->tiles().height(); y++) {
					Tile& tile = state->tile(x, y);
					if (tile.countryId >= 0) {
						Country& country = state->country(tile.countryId);
						switch (tile.building) {
						case BuildingType::Pasture:
							Enqueue<ChangeResourceAmountCommand>(country.id, ResourceType::Livestock, livestock_dist(random));
							break;
						case BuildingType::Field:
							Enqueue<ChangeResourceAmountCommand>(country.id, ResourceType::Wheat, wheat_dist(random));
							break;
						case BuildingType::Village:
						case BuildingType::Market:
						case BuildingType::Town:
							Enqueue<ChangeResourceAmountCommand>(country.id, ResourceType::Gold, static_cast<int>(tile.population * gold_dist(random)));
							Enqueue<GrowPopulationCommand>(x, y);
							break;
						}
					}
				}
			}
		}
	};

	export class EndRoundCommand : public Command {
	public:
		void Execute(std::shared_ptr<WorldState>& state, std::mt19937& random) override {
			Enqueue<ResetPopulationCountCommand>();
			Enqueue<FeedPopulationCommand>();
			Enqueue<IncomeCommand>();
			state->next_year();
		}
	};

	export class GameController {
	private:
		std::shared_ptr<WorldState> state_;
		Event<std::uint16_t> on_start_human_turn_;
		std::random_device rd_;
		std::mt19937 random_;
		std::queue<std::unique_ptr<Command>> command_queue_;

		void Execute(std::unique_ptr<Command> command) {
			command_queue_.push(std::move(command));

			while (!command_queue_.empty()) {
				std::unique_ptr<Command> next = std::move(command_queue_.front());
				command_queue_.pop();

				std::vector<std::unique_ptr<Command>> sub_commands = next->DoExecute(state_, random_);
				for (auto& cmd : sub_commands) {
					command_queue_.push(std::move(cmd));
				}
			}
		}

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
			Execute(std::make_unique<EndRoundCommand>());
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
