
#include "platform-opengl.h"

#include "sdl2-setup.h"
#include "input.h"
#include "ui/ui.h"
#include "log.h"
#include "players.h"
#include "font-icons.h"

#include "nanovg.h"
#ifdef _WIN32
#define NANOVG_GL3
#else
#define NANOVG_GLES3
#endif
#include "nanovg_gl.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "stb_image.h"
#include <gl.utilities.textures.h>
#include <gl.utilities.vertexbuffers.h>

enum class InputStates
{
    UiClicked,
    Idle,
    Panning,
    Rotating,
    Zooming
};

static const int BASELINE_WIDTH = 1920;
static const int BASELINE_HEIGHT = 1080;
static float screenScale = 1.0f;

class Program : public SDLProgram
{
public:
    Program(int width, int height);

    void handleInput();

    virtual bool SetUp();
    virtual void Render();
    virtual void CleanUp();
    virtual void OnResize(int width, int height);

    void moveCameraTo(Player* player);

    NVGcontext* vg;

    glm::mat4 _proj, _view;
    glm::vec3 _pos;
    Player* _target;
    InputStates _currentInputState;
    glm::vec2 _startMotion, _prevMotion;
    AnalogActionHandle _motionHandle;
    DigitalActionHandle _startPanningHandle;
    DigitalActionHandle _shootHandle;
};

static auto lastUIUpdateTime = 0.0f;

Program::Program(int width, int height)
    : SDLProgram(width, height), vg(nullptr), _target(nullptr),
      _currentInputState(InputStates::Idle),
      _motionHandle(0), _startPanningHandle(0), _shootHandle(0)
{ }

bool Program::SetUp()
{
#ifdef _WIN32
    this->vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#else
    this->vg = nvgCreateGLES3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#endif

    if (nvgCreateFont(this->vg, "icons", "entypo.ttf") == -1)
    {
        Log::Current().Error("Could not add font icons.\n");
        return false;
    }
    if (nvgCreateFont(this->vg, "fontawesome", "fontawesome-webfont.ttf") == -1)
    {
        Log::Current().Error("Could not add fontawesome.\n");
        return false;
    }
    if (nvgCreateFont(this->vg, "material-icons", "MaterialIcons-Regular.ttf") == -1)
    {
        Log::Current().Error("Could not add material-icons.\n");
        return false;
    }
    if (nvgCreateFont(this->vg, "sans", "Roboto-Regular.ttf") == -1)
    {
        Log::Current().Error("Could not add font italic.\n");
        return false;
    }
    if (nvgCreateFont(this->vg, "sans-bold", "Roboto-Bold.ttf") == -1)
    {
        Log::Current().Error("Could not add font bold.\n");
        return false;
    }

    this->_motionHandle = this->input()->getAnalogActionHandle("motion");
    this->_startPanningHandle = this->input()->getDigitalActionHandle("start_panning");
    this->_shootHandle = this->input()->getDigitalActionHandle("shoot");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    UI::Manager().init(this->input(), this->vg);

    Player::Manager().setup();
    Player::Manager()._level.load("de_dust");

    float buttonSize = this->height / 5.0f;

    auto label = new Label("lbl");
    label->setText("Radar-Strike");
    label->setSize(glm::vec2(72.0f));
    label->setPosition(glm::vec2(this->width / 2.0f, 40.0f));
    label->setColor(glm::vec4(255.0f, 255.0f, 255.0f, 255.0f));
    UI::Manager().addToGroup(GameModes::Play, label);

    auto ctPanel = new Panel("cts");
    ctPanel->setSize(glm::vec2(buttonSize + (buttonSize / 3.0f), this->height * 2));
    ctPanel->setPosition(glm::vec2(0.0f, 0.0f));
    ctPanel->setColor(glm::vec4(91.0f, 107.0f, 123.0f, 155.0f));
    UI::Manager().addToGroup(GameModes::Play, ctPanel);

    auto tPanel = new Panel("ts");
    tPanel->setSize(glm::vec2(buttonSize * 2, this->height * 2));
    tPanel->setPosition(glm::vec2(this->width - ((buttonSize / 3.0f) * 2.0f), 0.0f));
    tPanel->setColor(glm::vec4(91.0f, 107.0f, 123.0f, 155.0f));
    UI::Manager().addToGroup(GameModes::Play, tPanel);

    int tindex = 0, ctindex = 0;
    for (auto player : Player::Manager()._players)
    {
        auto playerButton = new PlayerButton(player->_name, player);
        playerButton->setSize(glm::vec2(buttonSize));
        playerButton->setText(player->_name);
        playerButton->setIcon(player->_team == Teams::CounterTerrorist ? eFontAwesomeIcons::FA_SHIELD : eFontAwesomeIcons::FA_BOMB);
        playerButton->setTextAlignment(eAlignments::Bottom);
        playerButton->onClick([this] (const Button* button) {
            this->moveCameraTo(((PlayerButton*)button)->player());
        });
        UI::Manager().addToGroup(GameModes::Play, playerButton);
        if (player->_team == Teams::Terrorist)
        {
            playerButton->setPosition(glm::vec2((buttonSize / 3.0f), (buttonSize / 3.0f) + (tindex * buttonSize)));
            tindex++;
        }
        else if (player->_team == Teams::CounterTerrorist)
        {
            playerButton->setPosition(glm::vec2(width - (buttonSize / 3.0f), (buttonSize / 3.0f) + (ctindex * buttonSize)));
            ctindex++;
        }
    }

    UI::Manager().changeGameMode(GameModes::Play);

    return true;
}

void Program::moveCameraTo(Player* player)
{
    Player::Manager().selectPlayer(player);
    this->_target = player;// = glm::vec3((this->width / 2) - player->_pos.x, (this->height / 2) - player->_pos.y, 0.0f);
}

void Program::Render()
{
    auto diff = this->elapsed() - lastUIUpdateTime;
    lastUIUpdateTime = this->elapsed();

    if (this->_target != nullptr)
    {
        auto targetPos = glm::vec3((this->width / 2) - this->_target->_pos.x, (this->height / 2) - this->_target->_pos.y, 0.0f);
        const float cameraSpeed = 5.0f;
        auto pos = glm::vec3(this->_view[3].x, this->_view[3].y, this->_view[3].z);
        auto todo = targetPos - pos;
        if (glm::length(todo) > 0.001f)
        {
            pos += todo * (diff * cameraSpeed);
        }
        else
        {
            pos = targetPos;
        }
        this->_view[3].x = pos.x;
        this->_view[3].y = pos.y;
    }

    UI::Manager().update(diff);
    while (!UI::Manager().clickedControls().empty())
    {
        const Button* btn = dynamic_cast<const Button*>(UI::Manager().clickedControls().back());
        if (btn != nullptr) btn->click();
        UI::Manager().clickedControls().pop();
    }
    this->handleInput();

    Player::Manager().update(diff);

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Player::Manager().render(this->_proj, this->_view);

    UI::Manager().render(this->width, this->height, screenScale);
}

static bool hasShot = false;

void Program::handleInput()
{
    if (UI::Manager().hoverControl() != nullptr)
    {
        this->_currentInputState = InputStates::UiClicked;
        return;
    }

    if (!hasShot && this->input()->getDigitalActionData(this->_shootHandle).state)
    {
        hasShot = true;
        Player::Manager().shoot();
    }
    else if (hasShot && !this->input()->getDigitalActionData(this->_shootHandle).state)
    {
        hasShot = false;
    }

    switch (this->_currentInputState)
    {
    case InputStates::Idle:
    {
        if (this->input()->getDigitalActionData(this->_startPanningHandle).state)
        {
            this->_currentInputState = InputStates::Panning;
            this->_startMotion = this->_prevMotion = glm::vec2(this->input()->getAnalogActionData(this->_motionHandle).x, this->input()->getAnalogActionData(this->_motionHandle).y);
        }
        break;
    }
    case InputStates::Panning:
    {
        glm::vec2 currentMotion = glm::vec2(this->input()->getAnalogActionData(this->_motionHandle).x, this->input()->getAnalogActionData(this->_motionHandle).y);;
        auto diff = glm::vec2(currentMotion.x - this->_prevMotion.x, currentMotion.y - this->_prevMotion.y);
        if (!this->input()->getDigitalActionData(this->_startPanningHandle).state)
        {
            if (glm::length(glm::vec2(currentMotion.x - this->_startMotion.x, currentMotion.y - this->_startMotion.y)) < 2.0f)
            {
                // We subtract the width width/height because the ortho view is centerd in the middle of the screen(see resize())
                glm::vec3 click(currentMotion.x, currentMotion.y, 0.0f);

                // We subtract the panning value from the view matrix
                auto viewPan = this->_view[3];
                click.x -= viewPan.x;
                click.y -= viewPan.y;

                Player::Manager().clickAt(click.x, click.y);
            }
            this->_currentInputState = InputStates::Idle;
        }
        else
        {
            if (this->input()->getDigitalActionData(this->_startPanningHandle).state &&
                    glm::length(glm::vec2(currentMotion.x - this->_startMotion.x, currentMotion.y - this->_startMotion.y)) > 2.0f)
            {
                // Disable tracking the selected player
                this->_target = nullptr;
            }
            this->_view = glm::translate(this->_view, glm::vec3(diff.x, diff.y, 0.0f));
            this->_prevMotion = currentMotion;
        }
        break;
    }
    case InputStates::Rotating:
    case InputStates::Zooming:
    {
        break;
    }
    case InputStates::UiClicked:
    {
        if (!this->input()->getDigitalActionData(this->_startPanningHandle).state)
        {
            this->_currentInputState = InputStates::Idle;
        }
        break;
    }
    }
}

void Program::OnResize(int width, int height)
{
    screenScale = width / float(BASELINE_WIDTH);
    glViewport(0, 0, width, height);

    this->_proj = glm::ortho(0.0f, float(width), float(height), 0.0f);
    this->_view = glm::translate(glm::mat4(1.0f), this->_pos + glm::vec3(0.0f, 0.0f, 0.1f));
}

void Program::CleanUp()
{ }

int main(int argc, char* argv[])
{
    return Program(BASELINE_WIDTH / 2, BASELINE_HEIGHT / 2).Run(argc, argv);
}
