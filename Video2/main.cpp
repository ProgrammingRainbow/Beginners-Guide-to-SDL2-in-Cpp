#include <SDL2/SDL.h>
#include <format>
#include <iostream>
#include <memory>

void initialize_sdl();
void close_sdl();

class Game {
  public:
    Game();

    void init();
    void run();

    static constexpr int width{800};
    static constexpr int height{600};

  private:
    const std::string title;
    SDL_Event event;

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer;
};

Game::Game()
    : title{"Close Window"}, window{nullptr, SDL_DestroyWindow},
      renderer{nullptr, SDL_DestroyRenderer} {}

void Game::init() {
    this->window.reset(SDL_CreateWindow(
        this->title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        this->width, this->height, SDL_WINDOW_SHOWN));
    if (!this->window) {
        auto error = std::format("Error creating window: {}", SDL_GetError());
        throw std::runtime_error(error);
    }

    this->renderer.reset(
        SDL_CreateRenderer(this->window.get(), -1, SDL_RENDERER_ACCELERATED));
    if (!this->renderer) {
        auto error = std::format("Error creating renderer: {}", SDL_GetError());
        throw std::runtime_error(error);
    }
}

void Game::run() {
    while (true) {
        while (SDL_PollEvent(&this->event)) {
            switch (event.type) {
            case SDL_QUIT:
                return;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_ESCAPE:
                    return;
                    break;
                default:
                    break;
                }
            default:
                break;
            }
        }

        SDL_RenderClear(this->renderer.get());

        SDL_RenderPresent(this->renderer.get());

        SDL_Delay(16);
    }
}

void initialize_sdl() {
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        auto error = std::format("Error initializing SDL: {}", SDL_GetError());
        throw std::runtime_error(error);
    }
}

void close_sdl() { SDL_Quit(); }

int main() {
    int exit_val = EXIT_SUCCESS;

    try {
        initialize_sdl();
        Game game;
        game.init();
        game.run();
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        exit_val = EXIT_FAILURE;
    }

    close_sdl();

    return exit_val;
}
