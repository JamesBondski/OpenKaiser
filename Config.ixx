export module Config;

import std;
import SDL3;
import General;
import WorldState;

namespace OpenKaiser {

	export class Config {
	private:
		static sdl::Color get_color(std::string& line) {
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
		 static std::vector<sdl::Color> load_country_colors() {
			std::vector<sdl::Color> country_colors;
			const std::string config_location = "data/config/country_colors.txt";
			std::ifstream config_file(config_location);
			// Check for errors
			if (config_file.bad()) {
				throw OpenKaiserError("Could not open Country Colors file in " + config_location);
			}

			std::string line;
			while (std::getline(config_file, line)) {
				country_colors.push_back(get_color(line));
			}
			return country_colors;
		}

		static std::unordered_map<TileType, sdl::Color> load_tile_colors() {
			const std::string config_location = "data/config/tile_colors.txt";
			std::ifstream config_file(config_location);
			// Check for errors
			if (config_file.bad()) {
				throw OpenKaiserError("Could not open Tile Colors file in " + config_location);
			}

			std::unordered_map<TileType, sdl::Color> tileColors;
			std::string line;
			std::getline(config_file, line);
			tileColors[TileType::Grass] = get_color(line);
			std::getline(config_file, line);
			tileColors[TileType::Water] = get_color(line);
			std::getline(config_file, line);
			tileColors[TileType::Mountain] = get_color(line);
			return tileColors;
		}
	};
}