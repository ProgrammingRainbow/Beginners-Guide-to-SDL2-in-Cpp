#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
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
    ~Game();

    void init();
    void run();
    void load_media();

    static constexpr int width{800};
    static constexpr int height{600};

  private:
    void update_text();
    void update_sprite();

    const std::string title;
    SDL_Event event;
    std::mt19937 gen;
    std::uniform_int_distribution<Uint8> rand_color;
    int font_size;
    SDL_Color font_color;
    std::string text_str;
    SDL_Rect text_rect;
    const int text_vel;
    int text_xvel;
    int text_yvel;
    SDL_Rect sprite_rect;
    const int sprite_vel;

    const Uint8 *keystate;

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer;
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> background;
    std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)> font;
    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> text_surf;
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> text;
    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> icon_surf;
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> sprite;
    std::unique_ptr<Mix_Chunk, decltype(&Mix_FreeChunk)> cpp_sound;
    std::unique_ptr<Mix_Chunk, decltype(&Mix_FreeChunk)> sdl_sound;
    std::unique_ptr<Mix_Music, decltype(&Mix_FreeMusic)> music;
};

Game::Game()
    : title{"Sound Effects and Music"}, gen{}, rand_color{0, 255},
      font_size{80}, font_color{255, 255, 255, 255}, text_str{"SDL"},
      text_rect{0, 0, 0, 0}, text_vel{3}, text_xvel{3}, text_yvel{3},
      sprite_rect{0, 0, 0, 0}, sprite_vel{5},
      keystate{SDL_GetKeyboardState(nullptr)},
      window{nullptr, SDL_DestroyWindow},
      renderer{nullptr, SDL_DestroyRenderer},
      background{nullptr, SDL_DestroyTexture}, font{nullptr, TTF_CloseFont},
      text_surf{nullptr, SDL_FreeSurface}, text{nullptr, SDL_DestroyTexture},
      icon_surf{nullptr, SDL_FreeSurface}, sprite{nullptr, SDL_DestroyTexture},
      cpp_sound{nullptr, Mix_FreeChunk}, sdl_sound{nullptr, Mix_FreeChunk},
      music{nullptr, Mix_FreeMusic} {}

Game::~Game() {
    Mix_HaltChannel(-1);
    Mix_HaltMusic();
}

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

    this->icon_surf.reset(IMG_Load("images/Cpp-logo.png"));
    if (!this->icon_surf) {
        auto error = std::format("Error loading Surface: {}", IMG_GetError());
        throw std::runtime_error(error);
    }
    SDL_SetWindowIcon(this->window.get(), this->icon_surf.get());

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

    this->sprite.reset(SDL_CreateTextureFromSurface(this->renderer.get(),
                                                    this->icon_surf.get()));
    if (!this->sprite) {
        auto error = std::format("Error creating Texture from Surface: {}",
                                 SDL_GetError());
        throw std::runtime_error(error);
    }

    if (SDL_QueryTexture(this->sprite.get(), nullptr, nullptr,
                         &this->sprite_rect.w, &this->sprite_rect.h)) {
        auto error = std::format("Error querying Texture: {}", SDL_GetError());
        throw std::runtime_error(error);
    }

    this->cpp_sound.reset(Mix_LoadWAV("sounds/Cpp.ogg"));
    if (!this->cpp_sound) {
        auto error = std::format("Error loading Chunk: {}", Mix_GetError());
        throw std::runtime_error(error);
    }

    this->sdl_sound.reset(Mix_LoadWAV("sounds/SDL.ogg"));
    if (!this->sdl_sound) {
        auto error = std::format("Error loading Chunk: {}", Mix_GetError());
        throw std::runtime_error(error);
    }

    this->music.reset(Mix_LoadMUS("music/freesoftwaresong-8bit.ogg"));
    if (!this->music) {
        auto error = std::format("Error loading Music: {}", Mix_GetError());
        throw std::runtime_error(error);
    }
}

void Game::update_text() {
    this->text_rect.x += this->text_xvel;
    this->text_rect.y += this->text_yvel;

    if (this->text_rect.x < 0) {
        this->text_xvel = this->text_vel;
        Mix_PlayChannel(-1, this->sdl_sound.get(), 0);
    } else if (this->text_rect.x + this->text_rect.w > this->width) {
        this->text_xvel = -this->text_vel;
        Mix_PlayChannel(-1, this->sdl_sound.get(), 0);
    }
    if (this->text_rect.y < 0) {
        this->text_yvel = this->text_vel;
        Mix_PlayChannel(-1, this->sdl_sound.get(), 0);
    } else if (this->text_rect.y + this->text_rect.h > this->height) {
        this->text_yvel = -this->text_vel;
        Mix_PlayChannel(-1, this->sdl_sound.get(), 0);
    }
}

void Game::update_sprite() {
    if (this->keystate[SDL_SCANCODE_LEFT] || this->keystate[SDL_SCANCODE_A]) {
        this->sprite_rect.x -= this->sprite_vel;
    }
    if (this->keystate[SDL_SCANCODE_RIGHT] || this->keystate[SDL_SCANCODE_D]) {
        this->sprite_rect.x += this->sprite_vel;
    }
    if (this->keystate[SDL_SCANCODE_UP] || this->keystate[SDL_SCANCODE_W]) {
        this->sprite_rect.y -= this->sprite_vel;
    }
    if (this->keystate[SDL_SCANCODE_DOWN] || this->keystate[SDL_SCANCODE_S]) {
        this->sprite_rect.y += this->sprite_vel;
    }
}

void Game::run() {
    if (Mix_PlayMusic(this->music.get(), -1)) {
        auto error = std::format("Error playing Music: {}", Mix_GetError());
        throw std::runtime_error(error);
    }

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
                    Mix_PlayChannel(-1, this->cpp_sound.get(), 0);
                    break;
                case SDL_SCANCODE_M:
                    if (Mix_PausedMusic()) {
                        Mix_ResumeMusic();
                    } else {
                        Mix_PauseMusic();
                    }
                    break;
                default:
                    break;
                }
            default:
                break;
            }
        }

        this->update_text();
        this->update_sprite();

        SDL_RenderClear(this->renderer.get());

        SDL_RenderCopy(this->renderer.get(), this->background.get(), nullptr,
                       nullptr);

        SDL_RenderCopy(this->renderer.get(), this->text.get(), nullptr,
                       &this->text_rect);
        SDL_RenderCopy(this->renderer.get(), this->sprite.get(), nullptr,
                       &this->sprite_rect);

        SDL_RenderPresent(this->renderer.get());

        SDL_Delay(16);
    }
}

void initialize_sdl() {
    int sdl_flags = SDL_INIT_EVERYTHING;
    int img_flags = IMG_INIT_PNG;
    int mix_flags = MIX_INIT_OGG;

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

    if ((Mix_Init(mix_flags) & mix_flags) != mix_flags) {
        auto error =
            std::format("Error initialize SDL_mixer: {}", Mix_GetError());
        throw std::runtime_error(error);
    }

    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,
                      MIX_DEFAULT_CHANNELS, 1024)) {
        auto error = std::format("Error Opening Audio: {}", Mix_GetError());
        throw std::runtime_error(error);
    }
}

void close_sdl() {
    Mix_CloseAudio();
    Mix_Quit();
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
