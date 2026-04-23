#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <string_view>
#include <stdexcept>

export module SDL3;

import std;

export namespace sdl {
    struct WindowDeleter {
        void operator()(SDL_Window* window) const {
            SDL_DestroyWindow(window);
        }
    };

    struct RendererDeleter {
        void operator()(SDL_Renderer* renderer) const {
            SDL_DestroyRenderer(renderer);
        }
    };

    struct TextureDeleter {
        void operator()(SDL_Texture* texture) const {
            SDL_DestroyTexture(texture);
        }
    };

    export using WindowPtr = std::unique_ptr<SDL_Window, WindowDeleter>;
    export using RendererPtr = std::unique_ptr<SDL_Renderer, RendererDeleter>;
    export using TexturePtr = std::shared_ptr<SDL_Texture>;

    export using WindowFlags = SDL_WindowFlags;
    export using Event = SDL_Event;
    export using FRect = SDL_FRect;
    
    export namespace EventType {
        export constexpr Uint32 Quit = SDL_EVENT_QUIT;
		export constexpr Uint32 KeyDown = SDL_EVENT_KEY_DOWN;
    }

    export class sdl_error : public std::runtime_error {
    public:
        sdl_error(const std::string& msg)
            : std::runtime_error(msg + ": " + SDL_GetError()) {}
    };

    export void init(SDL_InitFlags flags = SDL_INIT_VIDEO) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw sdl_error("Failed to initialize SDL");
        }
    }

    export WindowPtr create_window(std::string_view title, int w, int h, SDL_WindowFlags flags) {
        auto window = SDL_CreateWindow(title.data(), w, h, flags);
        if (!window) {
            throw sdl_error("Failed to create window");
        }
        return WindowPtr(window);
    }

    export RendererPtr create_renderer(SDL_Window* window) {
		auto renderer = SDL_CreateRenderer(window, nullptr);
        if(!renderer) {
            throw sdl_error("Failed to create renderer");
		}
        return RendererPtr(renderer);
    }

    export bool poll_event(Event& event) {
        return SDL_PollEvent(&event);
    }

    export void render_clear(RendererPtr& renderer) {
        if(!SDL_RenderClear(renderer.get())) {
            throw sdl_error("Failed to clear renderer");
		}
	}

    export void render_present(RendererPtr& renderer) {
        if(!SDL_RenderPresent(renderer.get())) {
            throw sdl_error("Failed to present renderer");
		}
	}

	export void render_debug_text(RendererPtr& renderer, float x, float y, std::string_view text) {
		if(!SDL_RenderDebugText(renderer.get(), x, y, text.data())) {
			throw sdl_error("Failed to render debug text");
        }
    }

    export void set_render_draw_color(RendererPtr& renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) {
        if(!SDL_SetRenderDrawColor(renderer.get(), r, g, b, alpha)) {
            throw sdl_error("Failed to set render draw color");
        }
	}

    export void render_fill_rect(RendererPtr& renderer, const FRect& rect) {
        if(!SDL_RenderFillRect(renderer.get(), &rect)) {
            throw sdl_error("Failed to fill rectangle");
        }
	}

	export void render_line(RendererPtr& renderer, float x1, float y1, float x2, float y2) {
        if(!SDL_RenderLine(renderer.get(), x1, y1, x2, y2)) {
            throw sdl_error("Failed to draw line");
        }
    }

    inline TexturePtr make_texture(SDL_Texture* texture) {
        return TexturePtr(texture, TextureDeleter{});
    }

    export TexturePtr load_texture(RendererPtr& renderer, std::string_view path) {
        SDL_Surface* surface = SDL_LoadPNG(path.data());
        if (!surface) {
            throw sdl_error(std::format("Failed to load image from {}", path));
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer.get(), surface);
        SDL_DestroySurface(surface);
        if (!texture) {
            throw sdl_error(std::format("Failed to create texture from {}", path));
        }
        return make_texture(texture);
    }

    export void render_texture(RendererPtr& renderer, TexturePtr& texture, const FRect& rect) {
        if (!SDL_RenderTexture(renderer.get(), texture.get(), NULL, &rect)) {
            throw sdl_error("Error rendering texture.");
        }
    }
}