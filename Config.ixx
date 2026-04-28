export module Config;

import std;
import SDL3;
import General;
import WorldState;

namespace OpenKaiser {

	export class Config {
	private:
		static sdl::Color GetColor(std::string& line) {
			std::stringstream splitter(line);
			int r, g, b;
			splitter >> r >> g >> b;

			if (!splitter.good()) {
				throw OpenKaiserError("Error parsing color: " + line);
			}

			return sdl::Color{
				static_cast<uint8_t>(r),
				static_cast<uint8_t>(g),
				static_cast<uint8_t>(b),
				255
			};
		}

	public:
		 static std::vector<sdl::Color> LoadCountryColors() {
			std::vector<sdl::Color> country_colors;
			const std::string kConfigLocation = "data/config/country_colors.txt";
			std::ifstream config_file(kConfigLocation);
			if (config_file.bad()) {
				throw OpenKaiserError("Could not open Country Colors file in " + kConfigLocation);
			}

			std::string line;
			while (std::getline(config_file, line)) {
				country_colors.push_back(GetColor(line));
			}
			return country_colors;
		}

		static std::unordered_map<TileType, sdl::Color> LoadTileColors() {
			const std::string kConfigLocation = "data/config/tile_colors.txt";
			std::ifstream config_file(kConfigLocation);
			if (config_file.bad()) {
				throw OpenKaiserError("Could not open Tile Colors file in " + kConfigLocation);
			}

			std::unordered_map<TileType, sdl::Color> tile_colors;
			std::string line;
			std::getline(config_file, line);
			tile_colors[TileType::Grass] = GetColor(line);
			std::getline(config_file, line);
			tile_colors[TileType::Water] = GetColor(line);
			std::getline(config_file, line);
			tile_colors[TileType::Mountain] = GetColor(line);
			return tile_colors;
		}
	};
}