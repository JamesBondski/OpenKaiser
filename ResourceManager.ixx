export module ResourceManager;

import std;
import SDL3;

namespace OpenKaiser {

	struct TextConfig {
		std::string text;
		float size;
		std::uint8_t r, g, b;

		bool operator==(const TextConfig& other) const {
			return text == other.text &&
				size == other.size &&
				r == other.r &&
				g == other.g &&
				b == other.b;
		}
	};

	struct TextConfigHash {
		std::size_t operator()(const TextConfig& config) const noexcept {
			std::size_t seed = 0;
			hash_combine(seed, config.text);
			hash_combine(seed, config.size);
			hash_combine(seed, config.r);
			hash_combine(seed, config.g);
			hash_combine(seed, config.b);
			return seed;
		}

	private:
		template <typename T>
		static void hash_combine(std::size_t& seed, const T& v) {
			seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};

	export class ResourceManager {
	private:
		sdl::RendererPtr renderer;

		// Images
		std::unordered_map<std::string, sdl::TexturePtr> textures;
		
		// Fonts
		std::unordered_map<float, sdl::FontPtr> fonts;
		std::unordered_map<TextConfig, sdl::TexturePtr, TextConfigHash> texts;
		
	public:
		void init(sdl::RendererPtr& renderer) {
			this->renderer = renderer;
		}

		sdl::TexturePtr& get_image(const std::string& path) {
			auto texture = textures.find(path);
			if (texture != textures.end()) {
				return texture->second;
			}

			auto [it, inserted] = textures.insert(std::pair<std::string, sdl::TexturePtr>(path, sdl::load_texture(this->renderer, path)));
			return it->second;
		}

		sdl::TexturePtr& get_text(const std::string& text, float size, std::uint8_t r, std::uint8_t g, std::uint8_t b ) {
			auto font_it = fonts.find(size);
			if (font_it == fonts.end()) {
				auto [it, inserted] = fonts.insert(std::pair<float, sdl::FontPtr>(size, sdl::ttf_open_font("data/fonts/OpenSans-Medium.ttf", size)));
				font_it = it;
			}

			TextConfig config = { text, size, r, g, b };
			auto text_it = texts.find(config);
			if (text_it != texts.end()) {
				return text_it->second;
			}
			else {
				sdl::Color color = { r, g, b, 255 };
				sdl::TexturePtr rendered = sdl::ttf_render_text(this->renderer, font_it->second, text, color);
				auto [it, inserted] = texts.insert(std::pair<TextConfig, sdl::TexturePtr>(config, rendered));
				return it->second;
			}
			
		}
	};

}