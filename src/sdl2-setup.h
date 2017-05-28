#ifndef SDL_SETUP_H
#define SDL_SETUP_H

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif
#include <SDL.h>
#include "input.h"

#include <string>
#include <vector>

class SDLProgram
{
public:
    SDLProgram(int width = 800, int height = 600);
    virtual ~SDLProgram();

    int Run(int argc, char* argv[]);

protected:
    virtual bool SetUp() = 0;
    virtual void Render() = 0;
    virtual void CleanUp() = 0;
    virtual void OnResize(int width, int height) = 0;

    const IInput* input() const;
    float elapsed() const;

    int width, height;
    std::string title;
    std::vector<std::string> args;
    bool keepRunning;

private:
    SDL_Window* _window;
    SDL_GLContext _context;
    Input _input;
    void handleInput(SDL_Event& event);

};

#endif // SDL_SETUP_H
