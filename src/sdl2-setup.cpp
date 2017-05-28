
#include "platform-opengl.h"

#include <SDL.h>
#include <iostream>

#include "sdl2-setup.h"
#include "log.h"

SDLProgram::SDLProgram(int width, int height)
    : width(width), height(height), title("SDLWindow"), keepRunning(true),
      _window(nullptr)
{
    Log::Current().Verbose("SDLProgram::SDLProgram(int width, int height)");
}

SDLProgram::~SDLProgram()
{ }

int SDLProgram::Run(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++)
    {
        this->args.push_back(argv[i]);
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
    {
        Log::Current().Error("Unable to initialize SDL");
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    this->_window = SDL_CreateWindow(title.c_str(),
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->width, this->height,
                                     SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
    if (this->_window == 0)
    {
        Log::Current().Error("Unable to create Window");
        return -1;
    }

    this->_context = SDL_GL_CreateContext(this->_window);
    if (this->_context == 0)
    {
        Log::Current().Error("Unable to create GL context");
        return -1;
    }

#ifdef _WIN32
    glExtLoadAll((PFNGLGETPROC*)SDL_GL_GetProcAddress);
#endif // _WIN32

    SDL_GetWindowSize(this->_window, &this->width, &this->height);
    this->OnResize(this->width, this->height);

    if (this->SetUp())
    {
        Log::Current().Info("Graphics initialized");

//        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_SetWindowGrab(this->_window, SDL_TRUE);
//        SDL_ShowCursor(SDL_FALSE);

        SDL_Event event;
        while (this->keepRunning)
        {
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_WINDOWEVENT)
                {
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    {
                        this->width = event.window.data1;
                        this->height = event.window.data2;

                        if (this->width <= 0) this->width = 1;
                        if (this->height <= 0) this->height = 1;

                        this->OnResize(this->width, this->height);
                    }
                    else if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                        this->keepRunning = false;
                }
                else
                {
                    this->handleInput(event);
                }
            }
            SDL_PumpEvents();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            this->Render();

            // Swap front and back rendering buffers
            SDL_GL_SwapWindow(this->_window);
        }
        SDL_DestroyWindow(this->_window);
        this->CleanUp();
    }

    SDL_Quit();

    return 0;
}

const IInput* SDLProgram::input() const
{
    return &this->_input;
}

float SDLProgram::elapsed() const
{
    return SDL_GetTicks() / 1000.0f;
}

void SDLProgram::handleInput(SDL_Event& event)
{
    if (event.type == SDL_MOUSEMOTION)
    {
        this->_input.OnMouseMove(double(event.motion.x), double(event.motion.y));
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
    {
        auto state = event.type == SDL_MOUSEBUTTONDOWN ? ActionStates::Pressed : ActionStates::Released;
        switch (event.button.button)
        {
        case SDL_BUTTON_LEFT:   this->_input.OnMouseAction(MouseButtons::Left, state); break;
        case SDL_BUTTON_RIGHT:  this->_input.OnMouseAction(MouseButtons::Right, state); break;
        case SDL_BUTTON_MIDDLE: this->_input.OnMouseAction(MouseButtons::Middle, state); break;
        case SDL_BUTTON_X1:     this->_input.OnMouseAction(MouseButtons::Button4, state); break;
        case SDL_BUTTON_X2:     this->_input.OnMouseAction(MouseButtons::Button5, state); break;
        }
    }
    else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
    {
        auto state = event.type == SDL_KEYDOWN ? ActionStates::Pressed : ActionStates::Released;
        if (event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z)
            this->_input.OnKeyAction(static_cast<KeyboardKeys>(int(KeyboardKeys::Character_A) + int(event.key.keysym.sym - SDLK_a)), state);
        else
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_BACKSPACE:    this->_input.OnKeyAction(KeyboardKeys::Backspace, state); break;
            case SDLK_RETURN:       this->_input.OnKeyAction(KeyboardKeys::Return, state); break;
            case SDLK_SPACE:        this->_input.OnKeyAction(KeyboardKeys::Character_Space, state); break;
            case SDLK_ESCAPE:       this->_input.OnKeyAction(KeyboardKeys::Escape, state); break;
            case SDLK_TAB:          this->_input.OnKeyAction(KeyboardKeys::Tab, state); break;
            }
        }
    }
}
