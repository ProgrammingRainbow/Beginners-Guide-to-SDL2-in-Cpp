#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <format>
#include <iostream>
#include <memory>
#include <random>

void initialize_sdl();
void close_sdl();

class Game {
  public:
    Game();

    void init();
    void run();
    void load_media();

    static constexpr int width{800};
    static constexpr int height{600};

  private:
    const std::string title;
    SDL_Event event;
    std::mt19937 gen;
    std::uniform_int_distribution<Uint8> rand_color;
    int font_size;
    SDL_Color font_color;
    std::string text_str;
    SDL_Rect text_rect;

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer;
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> background;
    std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)> font;
    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> text_surf;
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> text;
};

Game::Game()
    : title{"Create Text"}, gen{}, rand_color{0, 255}, font_size{80},
      font_color{255, 255, 255, 255}, text_str{"SDL"}, text_rect{0, 0, 0, 0},
      window{nullptr, SDL_DestroyWindow},
      renderer{nullptr, SDL_DestroyRenderer},
      background{nullptr, SDL_DestroyTexture}, font{nullptr, TTF_CloseFont},
      text_surf{nullptr, SDL_FreeSurface}, text{nullptr, SDL_DestroyTexture} {}

void Game::init() {
    this->window.reset(
        SDL_CreateWindow(this->title.c_str(), SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, this->width, this->height, 0));
    if (!this->window) {
        auto error = std::format("Error creating Window: {}", SDL_GetError());
        throw std::runtime_error(error);
    }

    this->renderer.reset(
        SDL_CreateRenderer(this->window.get(), -1, SDL_RENDERER_ACCELERATED));
    if (!this->renderer) {
        auto error = std::format("Error creating Renderer: {}", SDL_GetError());
        throw std::runtime_error(error);
    }

    this->gen.seed(std::random_device()());
}

void Game::load_media() {
    this->background.reset(
        IMG_LoadTexture(this->renderer.get(), "images/background.png"));
    if (!this->background) {
        auto error = std::format("Error loading Texture: {}", IMG_GetError());
        throw std::runtime_error(error);
    }

    this->font.reset(TTF_OpenFont("fonts/freesansbold.ttf", this->font_size));
    if (!this->font) {
        auto error = std::format("Error creating Font: {}", TTF_GetError());
        throw std::runtime_error(error);
    }

    this->text_surf.reset(TTF_RenderText_Blended(
        this->font.get(), this->text_str.c_str(), this->font_color));
    if (!this->text_surf) {
        auto error =
            std::format("Error loading text Surface: {}", TTF_GetError());
        throw std::runtime_error(error);
    }

    this->text_rect.w = this->text_surf->w;
    this->text_rect.h = this->text_surf->h;

    this->text.reset(SDL_CreateTextureFromSurface(this->renderer.get(),
                                                  this->text_surf.get()));
    if (!this->text) {
        auto error = std::format("Error creating Texture from Surface: {}",
                                 SDL_GetError());
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
                case SDL_SCANCODE_SPACE:
                    SDL_SetRenderDrawColor(this->renderer.get(),
                                           this->rand_color(this->gen),
                                           this->rand_color(this->gen),
                                           this->rand_color(this->gen), 255);
                    break;
                default:
                    break;
                }
            default:
                break;
            }
        }

        SDL_RenderClear(this->renderer.get());

        SDL_RenderCopy(this->renderer.get(), this->background.get(), nullptr,
                       nullptr);

        SDL_RenderCopy(this->renderer.get(), this->text.get(), nullptr,
                       &this->text_rect);

        SDL_RenderPresent(this->renderer.get());

        SDL_Delay(16);
    }
}

void initialize_sdl() {
    int sdl_flags = SDL_INIT_EVERYTHING;
    int img_flags = IMG_INIT_PNG;

    if (SDL_Init(sdl_flags)) {
        auto error = std::format("Error initialize SDL2: {}", SDL_GetError());
        throw std::runtime_error(error);
    }

    if ((IMG_Init(img_flags) & img_flags) != img_flags) {
        auto error =
            std::format("Error initialize SDL_image: {}", IMG_GetError());
        throw std::runtime_error(error);
    }

    if (TTF_Init()) {
        auto error =
            std::format("Error initialize SDL_ttf: {}", TTF_GetError());
        throw std::runtime_error(error);
    }
}

void close_sdl() {
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

int main() {
    int exit_val = EXIT_SUCCESS;

    try {
        initialize_sdl();
        Game game;
        game.init();
        game.load_media();
        game.run();
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        exit_val = EXIT_FAILURE;
    }

    close_sdl();

    return exit_val;
}
