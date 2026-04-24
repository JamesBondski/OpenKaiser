module;
#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <string_view>
#include <stdexcept>
#include <SDL3_TTF/SDL_ttf.h>

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
    export using RendererPtr = std::shared_ptr<SDL_Renderer>;
    export using TexturePtr = std::shared_ptr<SDL_Texture>;

    export using WindowFlags = SDL_WindowFlags;
    export using Event = SDL_Event;
    export using FRect = SDL_FRect;
    export using Color = SDL_Color;
    export using Point = SDL_Point;
    export using FPoint = SDL_FPoint;
    
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

    inline RendererPtr make_renderer_ptr(SDL_Renderer* renderer) {
        return RendererPtr(renderer, RendererDeleter{});
    }

    export RendererPtr create_renderer(SDL_Window* window) {
		auto renderer = SDL_CreateRenderer(window, nullptr);
        if(!renderer) {
            throw sdl_error("Failed to create renderer");
		}
        return make_renderer_ptr(renderer);
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

    export void set_render_draw_color(RendererPtr& renderer, Color color) {
        if(!SDL_SetRenderDrawColor(renderer.get(), color.r, color.g, color.b, color.a)) {
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

    inline TexturePtr make_texture_ptr(SDL_Texture* texture) {
        return TexturePtr(texture, TextureDeleter{});
    }

    export TexturePtr load_texture(RendererPtr& renderer, std::string_view path) {
        SDL_Surface* surface = SDL_LoadPNG(path.data());
        if (!surface) {
            throw sdl_error("Failed to load image from " + std::string(path));
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer.get(), surface);
        SDL_DestroySurface(surface);
        if (!texture) {
            throw sdl_error("Failed to create texture from " + std::string(path));
        }
        return make_texture_ptr(texture);
    }

    export void render_texture(RendererPtr& renderer, TexturePtr& texture, const FRect& rect) {
        if (!SDL_RenderTexture(renderer.get(), texture.get(), NULL, &rect)) {
            throw sdl_error("Error rendering texture.");
        }
    }

    export void render_texture_centered(RendererPtr& renderer, TexturePtr& texture, const FPoint& target) {
        FRect target_rect{ target.x - texture->w / 2, target.y - texture->h / 2, texture->w, texture->h };
        render_texture(renderer, texture, target_rect);

    }

    // SDL3_ttf
    struct FontDeleter {
        void operator()(TTF_Font* font) const {
            TTF_CloseFont(font);
        }
    };

    export using FontPtr = std::unique_ptr<TTF_Font, FontDeleter>;

    export void ttf_init() {
        if (!TTF_Init()) {
            throw sdl_error("Error initializing SDL3_ttf.");
        }
    }

    export FontPtr ttf_open_font(std::string_view file, float ptsize) {
        TTF_Font* font = TTF_OpenFont(file.data(), ptsize);
        if (!font) {
            throw sdl_error("Error loading font " + std::string(file));
        }

        return FontPtr(font);
    }

    export TexturePtr ttf_render_text(RendererPtr& renderer, FontPtr& font, std::string_view text, Color color) {
        SDL_Surface* surface = TTF_RenderText_Blended(font.get(), text.data(), 0, color);
        if (!surface) {
            throw sdl_error("Error rendering text " + std::string(text));
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer.get(), surface);
        SDL_DestroySurface(surface);
        if (!texture) {
            throw sdl_error("Failed to create texture from text " + std::string(text));
        }
        return make_texture_ptr(texture);
    }

    export std::uint64_t get_performance_counter() {
        return SDL_GetPerformanceCounter();
    }

    export std::uint64_t get_performance_frequency() {
        return SDL_GetPerformanceFrequency();
    }

    export void get_current_render_output_size(RendererPtr& renderer, int* w, int* h) {
        if (!SDL_GetCurrentRenderOutputSize(renderer.get(), w, h)) {
            throw sdl_error("Failed to get render output size.");
        }
    }
}