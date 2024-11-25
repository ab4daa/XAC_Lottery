#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "Candidate.h"
#include "Logging.h"
#include <cmath>

static const char* TITLE = "Class Slide";
static const float CANDIDATE_SPACE = 50.0f;
const float CANDITATE_SCREEN_H_PROPORTION = 0.25f;
const float WINNER_SCREEN_H_PROPORTION = 0.70f;
static const char* CARD_TEXTURE_PATH = "asset\\folded.jpg";

Slide::Slide(SDL_Window* window, SDL_Renderer* renderer, const char* image_path, SDL_Texture* back_texture)
    : window_(window), renderer_(renderer), back_texture_(back_texture)
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

Slide::~Slide()
{
    if (texture_ != NULL)
    {
        SDL_DestroyTexture(texture_);
    }
}

void Slide::Set_Position(float x, float y)
{
    x_ = x;
    y_ = y;
}

void Slide::Render()
{
    Render(0.0f, 0.0f);
}

void Slide::Render(float x_off, float y_off)
{
    if (window_ == NULL || texture_ == NULL || back_texture_ == NULL)
        return;

    SDL_FRect dst_rect;

    dst_rect.x = x_ + x_off;
    dst_rect.y = y_ + y_off;
    dst_rect.w = width_;
    dst_rect.h = height_;
    if (turn_back_)
    {
        SDL_RenderTextureTiled(renderer_, back_texture_, NULL, 1.0f, &dst_rect);
    }
    else
    {
        SDL_RenderTexture(renderer_, texture_, NULL, &dst_rect);
    }
}

bool Slide::Is_Out_Of_Window() const
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

void Slide::Get_Rect(SDL_FRect& r) const
{
    r.x = x_;
    r.y = y_;
    r.w = width_;
    r.h = height_;
}

void Slide::Init_Position()
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

//return true: winner animation end
bool Slide::Win(Uint64 elapse)
{
    elapse_ += elapse;

    if (window_ == NULL || texture_ == NULL)
        return true;

    int win_w, win_h;
    if (!SDL_GetWindowSize(window_, &win_w, &win_h))
    {
        Logging_Write("SDL_GetWindowSize failed: %s", SDL_GetError());
        return true;
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

    if (t >= 1.0f)
        return true;
    else
        return false;
}

void Slide::Update_Size_By_Window()
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

void Slide::Set_Turn_Back(bool turn_back)
{
    turn_back_ = turn_back;
}

Lottery_Slide_Show::Lottery_Slide_Show(SDL_Window* window, SDL_Renderer* renderer, std::vector<std::string>& candidate_files)
    : window_(window), renderer_(renderer), candidate_files_(candidate_files), generator_(rd_())
{
    back_texture_ = IMG_LoadTexture(renderer_, CARD_TEXTURE_PATH);
    if (back_texture_ == NULL)
    {
        char* msg = NULL;
        SDL_asprintf(&msg, "IMG_LoadTexture err: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, TITLE, msg, window_);
        Logging_Write(msg);
        SDL_free(msg);
        return;
    }
}

Lottery_Slide_Show::~Lottery_Slide_Show()
{
    if (back_texture_ != NULL)
    {
        SDL_DestroyTexture(back_texture_);
    }
}

void Lottery_Slide_Show::Run(Uint64 elapse)
{
    state_elapse_ += elapse;

    if (candidate_files_.empty())
        return ;

    // if right side of screen has space, add new candidate to run
    int win_w, win_h;
    if (!SDL_GetWindowSize(window_, &win_w, &win_h))
    {
        Logging_Write("SDL_GetWindowSize failed: %s", SDL_GetError());
        return ;
    }
    const int start_slow_cand_nb = 10;
    float most_right_edge = 0.0f;
    for (auto& it : slide_vec_)
    {
        SDL_FRect r;
        it->Get_Rect(r);
        if (r.x + r.w > most_right_edge)
            most_right_edge = r.x + r.w;
    }
    if (win_w > most_right_edge && (float)win_w - most_right_edge >= CANDIDATE_SPACE)
    {
        const char* new_candidate_file = candidate_files_[candidate_idx].c_str();
        std::shared_ptr<Slide> newSlide = std::make_shared<Slide>(window_, renderer_, new_candidate_file, back_texture_);
        if (state_ == Lottery_Slide_Show_State::FOLD_RUN)
            newSlide->Set_Turn_Back(true);

        if (state_ == Lottery_Slide_Show_State::SHOW_WINNER && winner_idx_ == candidate_idx)
            the_winner_ = newSlide;

        slide_vec_.push_back(newSlide);

        candidate_idx += 1;
        if (candidate_idx >= candidate_files_.size())
            candidate_idx = 0;
    }

    // move and show all candidates
    const float screen_time_normal = 5.0f;
    const float max_screen_time = 1.0f;
    float movement_per_sec;
    switch (state_)
    {
    case Lottery_Slide_Show_State::IDLE:
    {
        movement_per_sec = (float)win_w / screen_time_normal;
    }
        break;

    case Lottery_Slide_Show_State::FOLD_RUN:
    {
        const float acceleration = 200.0f;
        movement_per_sec = std::min((float)win_w / screen_time_normal + (float)state_elapse_ * acceleration / 1000.0f, (float)win_w / max_screen_time);
    }
        break;

    case Lottery_Slide_Show_State::SHOW_WINNER:
    {
        if (the_winner_)
        {
            const float screen_time_final = 10.0f;
            float s = (float)3.0f * state_elapse_ / 1000.0f;
            float screen_time_lerp = max_screen_time + s;
            if (screen_time_lerp > screen_time_final)
                screen_time_lerp = screen_time_final;
            movement_per_sec = (float)win_w / screen_time_lerp;
        }
        else
        {
            movement_per_sec = (float)win_w / max_screen_time;
        }
    }
        break;
    }

    bool win_animation_end = false;
    if (the_winner_)
    {
        SDL_FRect r;
        the_winner_->Get_Rect(r);
        if (r.x <= win_w / 2.0f)
        {
            stopped_ = true;
            win_animation_end = the_winner_->Win(elapse);
        }
    }
    std::vector<std::shared_ptr<Slide>>::iterator iter = slide_vec_.begin();
    while (iter != slide_vec_.end())
    {
        SDL_FRect r;
        (*iter)->Get_Rect(r);        
        if ((*iter)->Is_Out_Of_Window() == false)
        {
            switch (state_)
            {
            case Lottery_Slide_Show_State::IDLE:
                (*iter)->Set_Position(r.x - movement_per_sec * elapse / 1000.0f, r.y);
                (*iter)->Render(0.0f, abs(sin(r.x / 100.0f) * 50.0f));
                break;

            case Lottery_Slide_Show_State::FOLD_RUN:
                (*iter)->Set_Position(r.x - movement_per_sec * elapse / 1000.0f, r.y);
                (*iter)->Render();
                break;

            case Lottery_Slide_Show_State::SHOW_WINNER:
                if(!stopped_)
                    (*iter)->Set_Position(r.x - movement_per_sec * elapse / 1000.0f, r.y);
                (*iter)->Render();
                break;
            }            
            ++iter;
        }
        else
        {
            iter = slide_vec_.erase(iter);
        }
    }
    if (the_winner_)
        the_winner_->Render();

    // change state
    if (state_ == Lottery_Slide_Show_State::FOLD_RUN && state_elapse_ > fold_time_)
    {
        int max = candidate_files_.size();
        std::uniform_int_distribution<int> unif(0, max);
        winner_idx_ = unif(generator_);
        Logging_Write("Winner is %s", candidate_files_[winner_idx_].c_str());
        state_elapse_ = 0;
        state_ = Lottery_Slide_Show_State::SHOW_WINNER;
    }
    const bool* keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_RETURN])
    {
        switch (state_)
        {
        case Lottery_Slide_Show_State::IDLE:
        {
            if (state_elapse_ > 1000)
            {
                Logging_Write("Start lottery");
                std::uniform_int_distribution<Uint64> unif(6000, 9000);
                fold_time_ = unif(generator_);
                state_elapse_ = 0;
                state_ = Lottery_Slide_Show_State::FOLD_RUN;
            }
        }
            break;

        case Lottery_Slide_Show_State::SHOW_WINNER:
            if (win_animation_end)
            {
                // remove winner, so no one can win twice
                if (winner_idx_ != candidate_files_.size() - 1)
                {
                    candidate_files_[winner_idx_] = std::move(candidate_files_.back());
                }
                candidate_files_.pop_back();
                Logging_Write("Back to idle, remain %d candidates", candidate_files_.size());
                stopped_ = false;
                winner_idx_ = 0;
                slide_vec_.clear();
                the_winner_ = nullptr;
                state_elapse_ = 0;
                state_ = Lottery_Slide_Show_State::IDLE;
            }
            break;

        default:
            break;
        }
    }
}
