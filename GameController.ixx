export module GameController;

import WorldState;
import std;
import Events;
import WorldGenerator;
import Command;
import General;

namespace OpenKaiser {

	export class PlayerController {
	private:
		std::shared_ptr<WorldState> state_;

	public:
		void init(std::shared_ptr<WorldState>& state) {
			state_ = state;
		}

		std::vector<Coordinates> GetBuildableTiles() {
			std::vector<Coordinates> buildable;
			for (int x = 0; x < state_->tiles().width(); x++) {
				for (int y = 0; y < state_->tiles().height(); y++) {
					Coordinates coords{ x,y };
					Tile& tile = state_->tile(coords);
					if (tile.countryId == state_->current_country_id()) {
						auto neighbours = GetAdjacentTiles(coords);
						for (Coordinates neighbour_coords : neighbours) {
							if (state_->tile(neighbour_coords).countryId == -1
								&& std::ranges::find(buildable, neighbour_coords) == buildable.end()
								&& state_->tile(neighbour_coords).type != TileType::Water) {
								buildable.push_back(neighbour_coords);
							}
						}
					}
				}
			}
			return buildable;
		}

		bool CanBuild(Coordinates coords, BuildingType building) {
			Tile& tile = state_->tile(coords);
			return CanBuild(tile, building);
		}

		bool CanBuild(Tile& tile, BuildingType building) {
			// Can't build on owned tile
			if (tile.countryId != -1) {
				return false;
			}

			// Can't build over existing building
			if (tile.building != BuildingType::None) {
				return false;
			}

			// Can't afford it
			if (state_->current_country().resources[ResourceType::Gold] <= state_->rules().GetBuildingPrice(building)) {
				return false;
			}

			// No moves
			if (state_->current_country().builds_left == 0) {
				return false;
			}

			return true;
		}

		void Build(Coordinates coords, BuildingType building) {
			Tile& tile = state_->tile(coords);
			if (!CanBuild(tile, building)) {
				throw OpenKaiserError("Can't build on tile");
			}

			Country& current_country = state_->current_country();
			current_country.resources[ResourceType::Gold] -= state_->rules().GetBuildingPrice(building);
			current_country.builds_left--;
			tile.building = building;
			tile.countryId = state_->current_country_id();
		}
	};

	export class GameController {
	private:
		std::shared_ptr<WorldState> state_;
		Event<std::uint16_t> on_start_human_turn_;
		std::random_device rd_;
		std::mt19937 random_;
		std::list<std::unique_ptr<Command>> command_queue_;
		PlayerController player_;

		void Execute(std::unique_ptr<Command> command) {
			command_queue_.push_back(std::move(command));

			while (!command_queue_.empty()) {
				std::unique_ptr<Command> next = std::move(command_queue_.front());
				command_queue_.pop_front();

				std::vector<std::unique_ptr<Command>> sub_commands = next->DoExecute(state_, random_);
				for (auto& cmd : sub_commands) {
					command_queue_.push_front(std::move(cmd));
				}
			}
		}

	public:
		GameController()
			: random_(rd_()) {
		}

		PlayerController& player() {
			return player_;
		}

		Event<std::uint16_t>& on_start_human_turn() {
			return on_start_human_turn_;
		}

		std::shared_ptr<WorldState>& StartGame() {
			state_ = WorldGenerator().Generate(WorldConfig(), random_);
			player_.init(state_);
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
				while (state_->current_country().builds_left > 0 && state_->current_country().resources[ResourceType::Gold] > 150) {
					auto buildable_tiles = player_.GetBuildableTiles();
					if (buildable_tiles.size() == 0) {
						break;
					}
					std::uniform_int_distribution buildable_dist(0, (int)buildable_tiles.size() - 1);
					Coordinates coords = buildable_tiles[buildable_dist(random_)];
					Tile& tile = state_->tile(coords);
					BuildingType building = tile.type == TileType::Mountain ? BuildingType::Pasture : BuildingType::Field;
					player_.Build(coords, building);
				}
				EndTurn();
			}
		}
	};
}
