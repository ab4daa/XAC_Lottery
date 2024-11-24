
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include <vector>
#include <string>
#include <memory>
#include <random>
#include "Candidate.h"
#include "Logging.h"

enum class Lottery_State {
    IDLE,
    FOLD_RUN,
    SHOW_WINNER
};

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture* bg_texture = NULL;
static const char* TITLE = "XAC Lottery";
static const char* VERSION = "0.1";
static const char* CANDIDATE_DIR = "asset\\candidates";
static const char* BACKGROUIND_PATH = "asset\\background.png";
static std::vector<std::string> vec_candidates;
static Lottery_State state_ = Lottery_State::IDLE;
static std::shared_ptr<Candidate_Iterator> canditate_iter = nullptr;
static Uint64 last_tick_ = 0;
static bool change_state_ = false;
static int winner_idx = 0;


#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
int win_w;
int win_h;

std::random_device rd_;
std::default_random_engine generator_;
static void Random_init()
{
    generator_ = std::default_random_engine(rd_());
}

// generate random [min,max)
static int Random_int(int min, int max)
{
    std::uniform_int_distribution<int> unif(min, max);
    return unif(generator_);
}

static SDL_AppResult List_Files_To_Vector_(std::vector<std::string>& v, const char* path, const char* pattern)
{
    int filenames_cnt = 0;
    char** filenames = SDL_GlobDirectory(path, pattern, SDL_GLOB_CASEINSENSITIVE, &filenames_cnt);
    if (filenames == NULL)
    {
        char* msg = NULL;
        SDL_asprintf(&msg, "SDL_GlobDirectory err: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, msg, window);
        Logging_Write("%s", msg);
        SDL_free(msg);
        return SDL_APP_FAILURE;
    }

    for (int ii = 0; ii < filenames_cnt; ii++)
    {
        char* full_path = NULL;
        SDL_asprintf(&full_path, "%s\\%s", path, filenames[ii]);
        v.push_back(full_path);
        SDL_free(full_path);
    }

    SDL_free(filenames);
    return SDL_APP_CONTINUE;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    Logging_Init();
    Random_init();
    SDL_SetAppMetadata(TITLE, VERSION, TITLE);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        Logging_Write("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        Logging_Write("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetWindowFullscreen(window, true))
    {
        Logging_Write("Couldn't set fullscreen: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_GetWindowSize(window, &win_w, &win_h))
    {
        Logging_Write("Couldn't get win size: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        char* msg = NULL;
        SDL_asprintf(&msg, "SDL_image could not initialize! SDL_image Error: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, msg, window);
        Logging_Write("%s", msg);
        SDL_free(msg);
        return SDL_APP_FAILURE;
    }

    // load background
    bg_texture = IMG_LoadTexture(renderer, BACKGROUIND_PATH);
    if (bg_texture == NULL)
    {
        char* msg = NULL;
        SDL_asprintf(&msg, "IMG_LoadTexture err: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, msg, window);
        Logging_Write("%s", msg);
        SDL_free(msg);
        return SDL_APP_FAILURE;
    }

    //get all candidate files: png or jpg
    SDL_AppResult lfret = List_Files_To_Vector_(vec_candidates, CANDIDATE_DIR, "*.png");
    if (lfret != SDL_APP_CONTINUE)
        return lfret;

    lfret = List_Files_To_Vector_(vec_candidates, CANDIDATE_DIR, "*.jpg");
    if (lfret != SDL_APP_CONTINUE)
        return lfret;

    if (vec_candidates.size() < 10)
    {
        char* msg = NULL;
        SDL_asprintf(&msg, "Please put at least 10 images into %s", CANDIDATE_DIR);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, msg, window);
        Logging_Write("%s", msg);
        SDL_free(msg);
        return SDL_APP_FAILURE;
    }

    Logging_Write("Initially gather %d candidates", vec_candidates.size());

    canditate_iter = std::make_shared<Idle_Candidate_Iterator>(window, renderer, vec_candidates);    
    last_tick_ = SDL_GetTicks();
    Logging_Write("SDL_AppInit OK");
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    else if (event->type == SDL_EVENT_KEY_DOWN)
    {
        switch (event->key.key)
        {
        case SDLK_ESCAPE:
            return SDL_APP_SUCCESS;

        default:
            break;
        }
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    SDL_FRect dst_rect;
    const Uint64 now = SDL_GetTicks();
    const Uint64 elapsed = now - last_tick_;
    last_tick_ = now;

    if (change_state_)
    {
        switch (state_)
        {
        case Lottery_State::IDLE:
            canditate_iter = std::make_shared<Fold_Candidate_Iterator>(window, renderer, vec_candidates);
            state_ = Lottery_State::FOLD_RUN;
            Logging_Write("Start lottery");
            break;

        case Lottery_State::FOLD_RUN:
            winner_idx = Random_int(0, vec_candidates.size());
            Logging_Write("Winner is %s", vec_candidates[winner_idx].c_str());
            canditate_iter = std::make_shared<Winner_Candidate_Iterator>(window, renderer, vec_candidates, winner_idx);
            state_ = Lottery_State::SHOW_WINNER;
            break;

        case Lottery_State::SHOW_WINNER:
            // remove winner, so no one can win twice
            if (winner_idx != vec_candidates.size() - 1)
            {
                vec_candidates[winner_idx] = std::move(vec_candidates.back());
            }
            vec_candidates.pop_back();
            Logging_Write("Back to idle, remain %d candidates", vec_candidates.size());
            canditate_iter = std::make_shared<Idle_Candidate_Iterator>(window, renderer, vec_candidates);
            state_ = Lottery_State::IDLE;
            break;
        }
        change_state_ = false;
    }

    /* as you can see from this, rendering draws over whatever was drawn before it. */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  /* black, full alpha */
    SDL_RenderClear(renderer);  /* start with a blank canvas. */

    // backgroung
    dst_rect.x = 0.0f;
    dst_rect.y = 0.0f;
    dst_rect.w = (float)win_w;
    dst_rect.h = (float)win_h;
    SDL_RenderTexture(renderer, bg_texture, NULL, &dst_rect);

    if (canditate_iter->Run(elapsed) == false)
    {
        change_state_ = true;        
    }

    SDL_RenderPresent(renderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{    
    Logging_Write("SDL_AppQuit");
    canditate_iter = nullptr;
    SDL_DestroyTexture(bg_texture);
    /* SDL will clean up the window/renderer for us. */
    IMG_Quit();
    Logging_Close();
}

