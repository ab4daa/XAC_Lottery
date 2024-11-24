#ifndef __XAC_CANDIDATE_H__
#define __XAC_CANDIDATE_H__

#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include <memory>
#include <functional>


class Candidate {
public:
	Candidate(SDL_Window* window, SDL_Renderer* renderer, const char* image_path);
	~Candidate();
	void Set_Position(float x, float y);
	void Render();
	void Render(float x_off, float y_off);
	bool Is_Out_Of_Window() const;
	void Get_Rect(SDL_FRect& r) const;
	void Init_Position();
	void Win(Uint64 elapse);

protected:
	SDL_Window* window_{ NULL };
	SDL_Renderer* renderer_{ NULL };
	SDL_Texture* texture_{ NULL };
	float x_{ 0.0f };
	float y_{ 0.0f };
	float width_{ 0.0f };
	float height_{ 0.0f };
	int img_width_{ 0 };
	int img_height_{ 0 };
	Uint64 elapse_{ 0 };

	void Update_Size_By_Window();
};

// non virtual, cannot use base pointer
class Folded_Candidate : public Candidate {
public:
	Folded_Candidate(SDL_Window* window, SDL_Renderer* renderer, const char* image_path, SDL_Texture* folded_texture);
	~Folded_Candidate() = default;
	void Render();
	void Render(float x_off, float y_off);

private:
	SDL_Texture* folded_texture_{ NULL };
};

class Candidate_Iterator {
public:
	Candidate_Iterator(SDL_Window* window, SDL_Renderer* renderer, std::vector<std::string>& candidate_files);
	virtual ~Candidate_Iterator() = default;
	/*return false means everything runs out of screen, can be deleted*/
	virtual bool Run(Uint64 elapse) = 0;

protected:
	SDL_Window* window_{ NULL };
	SDL_Renderer* renderer_{ NULL };
	std::vector<std::string>& candidate_files_;

};

class Idle_Candidate_Iterator :public Candidate_Iterator {
public:
	Idle_Candidate_Iterator(SDL_Window* window, SDL_Renderer* renderer, std::vector<std::string>& candidate_files);
	~Idle_Candidate_Iterator() = default;
	bool Run(Uint64 elapse);

private:
	Uint64 elapse_{ 0 };
	size_t candidate_idx{ 0 };
	std::vector<std::shared_ptr<Candidate>> candidate_vec_;
};

class Fold_Candidate_Iterator :public Candidate_Iterator {
public:
	Fold_Candidate_Iterator(SDL_Window* window, SDL_Renderer* renderer, std::vector<std::string>& candidate_files);
	~Fold_Candidate_Iterator();
	bool Run(Uint64 elapse);

private:
	Uint64 total_elapsed{ 0 };
	SDL_Texture* folded_texture_{ NULL };
	size_t candidate_idx{ 0 };
	std::vector<std::shared_ptr<Folded_Candidate>> candidate_vec_;
};

class Winner_Candidate_Iterator :public Candidate_Iterator {
public:
	Winner_Candidate_Iterator(SDL_Window* window, SDL_Renderer* renderer, std::vector<std::string>& candidate_files, int winner_idx);
	~Winner_Candidate_Iterator() = default;
	bool Run(Uint64 elapse);

private:
	bool stopped_{ false };
	int winner_idx_{ 0 };
	size_t candidate_idx{ 0 };
	std::vector<std::shared_ptr<Candidate>> candidate_vec_;
	std::shared_ptr<Candidate> the_winner_;
};

#endif
