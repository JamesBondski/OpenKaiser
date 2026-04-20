#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <string_view>
#include <stdexcept>

export module SDL3;

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

    export using WindowPtr = std::unique_ptr<SDL_Window, WindowDeleter>;
    export using RendererPtr = std::unique_ptr<SDL_Renderer, RendererDeleter>;

    export using WindowFlags = SDL_WindowFlags;
    export using Event = SDL_Event;
    
    export namespace EventType {
        export constexpr Uint32 Quit = SDL_EVENT_QUIT;
    }

    export class sdl_error : public std::runtime_error {
    public:
        sdl_error(const std::string& msg)
            : std::runtime_error(msg + ": " + SDL_GetError()) {}
    };

    export bool init(SDL_InitFlags flags = SDL_INIT_VIDEO) {
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
}