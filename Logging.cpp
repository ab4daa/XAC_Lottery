#include <SDL3/SDL.h>
#include <ctime>

static const char* LOGGING_DIR = "log";
static SDL_IOStream* fLog = NULL;

bool Logging_Init()
{
	// create directory if not exist
	bool create_dir = false;
	SDL_PathInfo pi;
	if (SDL_GetPathInfo(LOGGING_DIR, &pi) == false)
	{
		create_dir = true;
	}
	else
	{
		if (pi.type != SDL_PATHTYPE_DIRECTORY)
			return false;
	}

	if (create_dir)
	{
		if (SDL_CreateDirectory(LOGGING_DIR) == false)
			return false;
	}

	// create log file for this session
	time_t timestamp;
	char time_str[64] = {0};
	struct tm* datetime;

	time(&timestamp);
	datetime = localtime(&timestamp);
	strftime(time_str, sizeof(time_str), "%F-%H-%M-%S", datetime);
	char* Log_path = NULL;
	SDL_asprintf(&Log_path, "%s\\session-%s.log", LOGGING_DIR, time_str) ;
	fLog = SDL_IOFromFile(Log_path, "w");
	SDL_free(Log_path);
	if (fLog == NULL)
	{		
		return false;
	}
	return true;
}

bool Logging_Write(const char* fmt, ...)
{
	if (fLog == NULL)
		return false;

	time_t timestamp;
	char time_str[64] = { 0 };
	struct tm* datetime;

	time(&timestamp);
	datetime = localtime(&timestamp);
	strftime(time_str, sizeof(time_str), "%F-%X", datetime);

	va_list argptr;
	va_start(argptr, fmt);
	char* Log_msg = NULL;
	SDL_vasprintf(&Log_msg, fmt, argptr);
	va_end(argptr);

	SDL_IOprintf(fLog, "%s: %s\r\n", time_str, Log_msg);

	SDL_free(Log_msg);

	return true;
}

bool Logging_Close()
{
	if (fLog == NULL)
		return false;

	return SDL_CloseIO(fLog);
}

