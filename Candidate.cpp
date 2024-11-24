#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "Candidate.h"
#include "Logging.h"
#include <cmath>

static const char* TITLE = "Class Candidate";
static const float CANDIDATE_SPACE = 50.0f;
const float CANDITATE_SCREEN_H_PROPORTION = 0.25f;
const float WINNER_SCREEN_H_PROPORTION = 0.70f;
static const char* CARD_TEXTURE_PATH = "asset\\0021-1024x1024.jpg";

Candidate::Candidate(SDL_Window* window, SDL_Renderer* renderer, const char* image_path)
	: renderer_(renderer), window_(window)
{
    texture_ = IMG_LoadTexture(renderer_, image_path);
    if (texture_ == NULL)
    {
        char* msg = NULL;
        SDL_asprintf(&msg, "IMG_LoadTexture err: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, msg, window_);
        Logging_Write(msg);
        SDL_free(msg);
        return;
    }
    img_width_ = texture_->w;
    img_height_ = texture_->h;

    Init_Position();
}

Candidate::~Candidate()
{
    if (texture_ != NULL)
    {
        SDL_DestroyTexture(texture_);
    }
}

void Candidate::Update_Size_By_Window()
{
    if (window_ == NULL || texture_ == NULL)
        return;

    // scale candidate image by height proportionally according to window size
    int win_w, win_h;
    if (!SDL_GetWindowSize(window_, &win_w, &win_h))
    {
        Logging_Write("SDL_GetWindowSize failed: %s", SDL_GetError());
        return;
    }

    float img_w_h_ratio = (float)img_width_ / (float)img_height_;
    height_ = ((float)win_h) * CANDITATE_SCREEN_H_PROPORTION;
    width_ = img_w_h_ratio * height_;
}

void Candidate::Set_Position(float x, float y)
{
    x_ = x;
    y_ = y;
}

void Candidate::Render()
{
    Render(0.0f, 0.0f);
}

void Candidate::Render(float x_off, float y_off)
{
    if (window_ == NULL || texture_ == NULL)
        return;

    SDL_FRect dst_rect;

    dst_rect.x = x_ + x_off;
    dst_rect.y = y_ + y_off;
    dst_rect.w = width_;
    dst_rect.h = height_;
    SDL_RenderTexture(renderer_, texture_, NULL, &dst_rect);
}

bool Candidate::Is_Out_Of_Window() const
{
    if (window_ == NULL || texture_ == NULL)
        return true;

    // scale candidate image by height proportionally according to window size
    int win_w, win_h;
    if (!SDL_GetWindowSize(window_, &win_w, &win_h))
    {
        Logging_Write("SDL_GetWindowSize failed: %s", SDL_GetError());
        return true;
    }

    SDL_FRect screen_rect;
    screen_rect.x = 0.0f;
    screen_rect.y = 0.0f;
    screen_rect.w = (float)win_w;
    screen_rect.h = (float)win_h;

    SDL_FRect can_rect;
    can_rect.x = x_;
    can_rect.y = y_;
    can_rect.w = width_;
    can_rect.h = height_;

    if (SDL_HasRectIntersectionFloat(&screen_rect, &can_rect))
        return false;
    else
        return true;
}

void Candidate::Get_Rect(SDL_FRect& r) const
{
    r.x = x_;
    r.y = y_;
    r.w = width_;
    r.h = height_;
}

void Candidate::Init_Position()
{
    if (window_ == NULL || texture_ == NULL)
        return;

    Update_Size_By_Window();
    //initial position
    int win_w, win_h;
    if (!SDL_GetWindowSize(window_, &win_w, &win_h))
    {
        Logging_Write("SDL_GetWindowSize failed: %s", SDL_GetError());
        return;
    }
    x_ = (float)win_w;
    y_ = ((float)win_h - height_) / 2.0f;
}

void Candidate::Win(Uint64 elapse)
{
    elapse_ += elapse;

    if (window_ == NULL || texture_ == NULL)
        return;

    int win_w, win_h;
    if (!SDL_GetWindowSize(window_, &win_w, &win_h))
    {
        Logging_Write("SDL_GetWindowSize failed: %s", SDL_GetError());
        return;
    }
    const float img_w_h_ratio = (float)img_width_ / (float)img_height_;
    float init_height_ = ((float)win_h) * CANDITATE_SCREEN_H_PROPORTION;
    float init_width_ = img_w_h_ratio * init_height_;
    float end_height_ = ((float)win_h) * WINNER_SCREEN_H_PROPORTION;
    float end_width_ = img_w_h_ratio * end_height_;
    float t = 1.0f;
    const Uint64 animate_time = 1 * 1000;
    if (elapse_ < animate_time)
    {
        t = (float)elapse_ / animate_time;
    }

    width_ = init_width_ + (end_width_ - init_width_) * t;
    height_ = init_height_ + (end_height_ - init_height_) * t;
    x_ = (float)(win_w - width_) / 2.0f;
    y_ = ((float)win_h - height_) / 2.0f;
}

Candidate_Iterator::Candidate_Iterator(SDL_Window* window, SDL_Renderer* renderer, std::vector<std::string>& candidate_files)
    : renderer_(renderer), window_(window), candidate_files_(candidate_files)
{
}

Idle_Candidate_Iterator::Idle_Candidate_Iterator(SDL_Window* window, SDL_Renderer* renderer, std::vector<std::string>& candidate_files)
    : Candidate_Iterator(window, renderer, candidate_files)
{
}

bool Idle_Candidate_Iterator::Run(Uint64 elapse)
{
    elapse_ += elapse;

    if (candidate_files_.empty())
        return false;

    // if right side of screen has space, add new candidate to run
    int win_w, win_h;
    if (!SDL_GetWindowSize(window_, &win_w, &win_h))
    {
        Logging_Write("SDL_GetWindowSize failed: %s", SDL_GetError());
        return false;
    }
    float most_right_edge = 0.0f;
    for (auto& it : candidate_vec_)
    {
        SDL_FRect r;
        it->Get_Rect(r);
        if (r.x + r.w > most_right_edge)
            most_right_edge = r.x + r.w;
    }
    if (win_w > most_right_edge && (float)win_w - most_right_edge >= CANDIDATE_SPACE)
    {
        const char* new_candidate_file = candidate_files_[candidate_idx].c_str();
        candidate_idx += 1;
        if (candidate_idx >= candidate_files_.size())
            candidate_idx = 0;
        candidate_vec_.push_back(std::make_shared<Candidate>(window_, renderer_, new_candidate_file));
    }

    // move and show all candidates
    const float screen_time = 5.0f;
    const float movement_per_sec = (float)win_w / screen_time;
    std::vector<std::shared_ptr<Candidate>>::iterator iter = candidate_vec_.begin();
    while (iter != candidate_vec_.end())
    {
        SDL_FRect r;
        (*iter)->Get_Rect(r);
        (*iter)->Set_Position(r.x - movement_per_sec * elapse / 1000.0f, r.y);
        if ((*iter)->Is_Out_Of_Window() == false)
        {
            (*iter)->Render(0.0f, abs(sin(r.x / 100.0f)*50.0f));
            ++iter;
        }
        else
        {
            iter = candidate_vec_.erase(iter);
        }
    }

    const bool* keystate = SDL_GetKeyboardState(NULL);
    if (elapse_ >= 1000 && keystate[SDL_SCANCODE_RETURN])
        return false;

    return true;
}

Folded_Candidate::Folded_Candidate(SDL_Window* window, SDL_Renderer* renderer, const char* image_path, SDL_Texture* folded_texture)
    : Candidate(window, renderer, image_path)
{ 
    folded_texture_ = folded_texture;
}

void Folded_Candidate::Render(float x_off, float y_off)
{
    if (window_ == NULL || folded_texture_ == NULL)
        return;

    SDL_FRect dst_rect;

    dst_rect.x = x_ + x_off;
    dst_rect.y = y_ + y_off;
    dst_rect.w = width_;
    dst_rect.h = height_;
    SDL_RenderTextureTiled(renderer_, folded_texture_, NULL, 1.0f, &dst_rect);
}

void Folded_Candidate::Render()
{
    Render(0.0f, 0.0f);
}

Fold_Candidate_Iterator::Fold_Candidate_Iterator(SDL_Window* window, SDL_Renderer* renderer, std::vector<std::string>& candidate_files)
    : Candidate_Iterator(window, renderer, candidate_files)
{
    folded_texture_ = IMG_LoadTexture(renderer_, CARD_TEXTURE_PATH);
    if (folded_texture_ == NULL)
    {
        char* msg = NULL;
        SDL_asprintf(&msg, "IMG_LoadTexture err: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, msg, window_);
        Logging_Write(msg);
        SDL_free(msg);
        return;
    }
}

Fold_Candidate_Iterator::~Fold_Candidate_Iterator()
{
    if (folded_texture_ != NULL)
    {
        SDL_DestroyTexture(folded_texture_);
    }
}

bool Fold_Candidate_Iterator::Run(Uint64 elapse)
{
    const Uint64 FOLDED_MAX_RUN_TIME = 8 * 1000;

    total_elapsed += elapse;
    if (candidate_files_.empty())
        return false;

    // if right side of screen has space, add new candidate to run
    int win_w, win_h;
    if (!SDL_GetWindowSize(window_, &win_w, &win_h))
    {
        Logging_Write("SDL_GetWindowSize failed: %s", SDL_GetError());
        return false;
    }
    float most_right_edge = 0.0f;
    for (auto& it : candidate_vec_)
    {
        SDL_FRect r;
        it->Get_Rect(r);
        if (r.x + r.w > most_right_edge)
            most_right_edge = r.x + r.w;
    }
    if (win_w > most_right_edge && (float)win_w - most_right_edge >= CANDIDATE_SPACE)
    {
        if (candidate_idx < candidate_files_.size())
        {
            const char* new_candidate_file = candidate_files_[candidate_idx].c_str();
            candidate_vec_.push_back(std::make_shared<Folded_Candidate>(window_, renderer_, new_candidate_file, folded_texture_));
            candidate_idx += 1;
        }
    }

    // move and show all candidates
    const float screen_time = 5.0f;
    const float max_screen_time = 1.0f;
    const float acceleration = 200.0f;
    const float movement_per_sec = std::min((float)win_w / screen_time + (float)total_elapsed * acceleration / 1000.0f, (float)win_w / max_screen_time);
    std::vector<std::shared_ptr<Folded_Candidate>>::iterator iter = candidate_vec_.begin();
    while (iter != candidate_vec_.end())
    {
        SDL_FRect r;
        (*iter)->Get_Rect(r);
        (*iter)->Set_Position(r.x - movement_per_sec * elapse / 1000.0f, r.y);
        if ((*iter)->Is_Out_Of_Window() == false)
        {
            (*iter)->Render();
            ++iter;
        }
        else
        {
            iter = candidate_vec_.erase(iter);
        }
    }

    if ((total_elapsed > 3000 && candidate_vec_.empty()) || total_elapsed > FOLDED_MAX_RUN_TIME)
        return false;
    else
        return true;
}

Winner_Candidate_Iterator::Winner_Candidate_Iterator(SDL_Window* window, SDL_Renderer* renderer, std::vector<std::string>& candidate_files, int winner_idx)
    : Candidate_Iterator(window, renderer, candidate_files), winner_idx_(winner_idx)
{
}

bool Winner_Candidate_Iterator::Run(Uint64 elapse)
{
    if (candidate_files_.empty())
        return false;

    // if right side of screen has space, add new candidate to run
    int win_w, win_h;
    if (!SDL_GetWindowSize(window_, &win_w, &win_h))
    {
        return false;
    }
    float most_right_edge = 0.0f;
    for (auto& it : candidate_vec_)
    {
        SDL_FRect r;
        it->Get_Rect(r);
        if (r.x + r.w > most_right_edge)
            most_right_edge = r.x + r.w;
    }
    if (win_w > most_right_edge && (float)win_w - most_right_edge >= CANDIDATE_SPACE)
    {
        if (candidate_idx < candidate_files_.size())
        {
            const char* new_candidate_file = candidate_files_[candidate_idx].c_str();
            std::shared_ptr<Candidate> new_candidate = std::make_shared<Candidate>(window_, renderer_, new_candidate_file);
            if (candidate_idx == winner_idx_)
                the_winner_ = new_candidate;

            candidate_vec_.push_back(new_candidate);
            candidate_idx += 1;
        }
    }

    // move and show all candidates
    const float screen_time = 1.0f;
    float movement_per_sec = (float)win_w / screen_time;
    // if running close to winner, slow down
    const int start_slow_cand_nb = 10;
    const float screen_time_final = 10.0f;
    if (candidate_idx + start_slow_cand_nb > winner_idx_ )
    {
        if (candidate_idx >= winner_idx_)
        {
            movement_per_sec = (float)win_w / screen_time_final;
        }
        else
        {
            float lerp = 1.0f - (float)(winner_idx_ - candidate_idx) / (float)start_slow_cand_nb;
            float screen_time_lerp = (screen_time_final - screen_time) * lerp + screen_time;
            movement_per_sec = (float)win_w / screen_time_lerp;
        }
    }
    std::vector<std::shared_ptr<Candidate>>::iterator iter = candidate_vec_.begin();
    while (iter != candidate_vec_.end())
    {
        if (!stopped_)
        {
            SDL_FRect r;
            (*iter)->Get_Rect(r);
            (*iter)->Set_Position(r.x - movement_per_sec * elapse / 1000.0f, r.y);
        }

        if ((*iter)->Is_Out_Of_Window() == false)
        {
            (*iter)->Render();
            ++iter;
        }
        else
        {
            iter = candidate_vec_.erase(iter);
        }
    }

    if (the_winner_)
    {
        SDL_FRect r;
        the_winner_->Get_Rect(r);
        if (r.x <= win_w / 2)
        {
            stopped_ = true;
            the_winner_->Win(elapse);
            the_winner_->Render();
        }
    }

    const bool* keystate = SDL_GetKeyboardState(NULL);
    if (stopped_ && keystate[SDL_SCANCODE_RETURN])
        return false;

    return true;
}
