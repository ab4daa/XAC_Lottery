#ifndef __XAC_CANDIDATE_H__
#define __XAC_CANDIDATE_H__

#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include <memory>
#include <random>

class Slide {
public:
	Slide(SDL_Window* window, SDL_Renderer* renderer, const char* image_path, SDL_Texture* back_texture);
	~Slide();
	void Set_Position(float x, float y);
	void Render();
	void Render(float x_off, float y_off);
	bool Is_Out_Of_Window() const;
	void Get_Rect(SDL_FRect& r) const;
	void Init_Position();
	bool Win(Uint64 elapse);
	void Set_Turn_Back(bool turn_back);

protected:
	SDL_Window* window_{ NULL };
	SDL_Renderer* renderer_{ NULL };
	SDL_Texture* texture_{ NULL };
	SDL_Texture* back_texture_{ NULL };
	float x_{ 0.0f };
	float y_{ 0.0f };
	float width_{ 0.0f };
	float height_{ 0.0f };
	int img_width_{ 0 };
	int img_height_{ 0 };
	Uint64 elapse_{ 0 };
	bool turn_back_{ false };

	void Update_Size_By_Window();
};

enum class Lottery_Slide_Show_State {
	IDLE,
	FOLD_RUN,
	SHOW_WINNER
};

class Lottery_Slide_Show {
public:
	Lottery_Slide_Show(SDL_Window* window, SDL_Renderer* renderer, std::vector<std::string>& candidate_files);
	~Lottery_Slide_Show();
	void Run(Uint64 elapse);

private:
	int winner_idx_{ 0 };
	std::random_device rd_;
	std::default_random_engine generator_;
	Lottery_Slide_Show_State state_{ Lottery_Slide_Show_State::IDLE };
	SDL_Window* window_{ NULL };
	SDL_Renderer* renderer_{ NULL };
	SDL_Texture* back_texture_{ NULL };
	std::vector<std::string>& candidate_files_;
	Uint64 state_elapse_{ 0 };
	Uint64 fold_time_{ 0 };
	size_t candidate_idx{ 0 };
	std::vector<std::shared_ptr<Slide>> slide_vec_;
	std::shared_ptr<Slide> the_winner_;
	bool stopped_{ false };
};

#endif
