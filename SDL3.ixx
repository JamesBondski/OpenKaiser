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
        return WindowPtr(SDL_CreateWindow(title.data(), w, h, flags));
    }

    export RendererPtr create_renderer(SDL_Window* window, std::string_view name) {
        return RendererPtr(SDL_CreateRenderer(window, name.data()));
    }

    export bool poll_event(Event& event) {
                return SDL_PollEvent(&event);
    }
}