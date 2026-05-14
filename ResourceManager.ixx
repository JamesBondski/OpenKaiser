module;
#include <plog/Log.h>

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
		sdl::RendererPtr renderer_;

		std::unordered_map<std::string, sdl::TexturePtr> textures_;

		std::unordered_map<float, sdl::FontPtr> fonts_;
		std::unordered_map<TextConfig, sdl::TexturePtr, TextConfigHash> texts_;

	public:
		void Init(sdl::RendererPtr& renderer) {
			renderer_ = renderer;
		}

		void Update(float time_passed) {
			std::vector<std::string> remove;
			for (auto& texture : textures_) {
				if (texture.second.use_count() == 1) {
					remove.push_back(texture.first);
				}
			}

			for (auto name : remove) {
				PLOG_DEBUG << "Removing texture " << name;
				textures_.erase(name);
			}

			std::vector<TextConfig> remove_text;
			for (auto& text : texts_) {
				if (text.second.use_count() == 1) {
					remove_text.push_back(text.first);
				}
			}

			for (auto text_info : remove_text) {
				PLOG_DEBUG << "Removing text " << text_info.text << " (size " << text_info.size << ").";
				texts_.erase(text_info);
			}
		}

		sdl::TexturePtr& get_image(const std::string& path) {
			auto texture = textures_.find(path);
			if (texture != textures_.end()) {
				return texture->second;
			}
			PLOG_DEBUG << "Loading image " << path;
			auto [it, inserted] = textures_.insert(std::pair<std::string, sdl::TexturePtr>(path, sdl::load_texture(renderer_, path)));
			return it->second;
		}

		sdl::TexturePtr& get_text(const std::string& text, float size, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
			sdl::Color color = { r, g, b, 255 };
			return get_text(text, size, color);
		}

		sdl::TexturePtr& get_text(const std::string& text, float size, sdl::Color color) {
			auto font_it = fonts_.find(size);
			if (font_it == fonts_.end()) {
				auto [it, inserted] = fonts_.insert(std::pair<float, sdl::FontPtr>(size, sdl::ttf_open_font("data/fonts/OpenSans-Medium.ttf", size)));
				font_it = it;
			}

			TextConfig config = { text, size, color.r, color.g, color.b };
			auto text_it = texts_.find(config);
			if (text_it != texts_.end()) {
				return text_it->second;
			}
			else {
				PLOG_DEBUG << "Rendering text '" << text << "' (size " << size << "), RGB(" << color.r << "," << color.g << "," << color.b << ")";
				sdl::TexturePtr rendered = sdl::ttf_render_text(renderer_, font_it->second, text, color);
				auto [it, inserted] = texts_.insert(std::pair<TextConfig, sdl::TexturePtr>(config, rendered));
				return it->second;
			}
		}
	};

}