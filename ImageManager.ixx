export module ImageManager;

import std;
import SDL3;

namespace OpenKaiser {

	export class ImageManager {
	private:
		std::unordered_map<std::string, sdl::TexturePtr> textures;

	public:
		sdl::TexturePtr& get_image(sdl::RendererPtr& renderer, const std::string& path) {
			auto texture = textures.find(path);
			if (texture != textures.end()) {
				return texture->second;
			}

			auto [it, inserted] = textures.insert(std::pair<std::string, sdl::TexturePtr>(path, sdl::load_texture(renderer, path)));
			return it->second;
		}
	};

}