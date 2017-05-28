#include "ui.h"
#include "../input.h"

#include "../platform-opengl.h"
#include "nanovg.h"
#ifdef _WIN32
#define NANOVG_GL3
#else
#define NANOVG_GLES3
#endif
#include "nanovg_gl.h"
#include "../font-icons.h"
#include "../players.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#define ICON_SEARCH 0x1F50D
#define ICON_CIRCLED_CROSS 0x2716
#define ICON_CHEVRON_RIGHT 0xE75E
#define ICON_CHECK 0x2713
#define ICON_LOGIN 0xE740
#define ICON_TRASH 0xE729
#define ICON_RETRY 0x27F2

static char* cpToUTF8(int cp, char* str)
{
    int n = 0;
    if (cp < 0x80) n = 1;
    else if (cp < 0x800) n = 2;
    else if (cp < 0x10000) n = 3;
    else if (cp < 0x200000) n = 4;
    else if (cp < 0x4000000) n = 5;
    else if (cp <= 0x7fffffff) n = 6;
    str[n] = '\0';
    switch (n) {
    case 6: str[5] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x4000000;
    case 5: str[4] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x200000;
    case 4: str[3] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x10000;
    case 3: str[2] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x800;
    case 2: str[1] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0xc0;
    case 1: str[0] = cp;
    }
    return str;
}

UI* UI::_instance = nullptr;

UI::UI()
    : _hoverControl(nullptr), _clickControl(nullptr)
{ }

UI& UI::Manager()
{
    if (UI::_instance == nullptr) UI::_instance = new UI();

    return *UI::_instance;
}

UI::~UI()
{ }

void UI::init(const IInput* input, NVGcontext* vg)
{
    this->vg = vg;
    this->_input = input;
    this->_swipeHandle = this->_input->getAnalogActionHandle("motion");
    this->_startSwipingHandle = this->_input->getDigitalActionHandle("start_panning");

}

void UI::update(float elapsed)
{
    for (auto control : this->_controls)
        control->update(elapsed);

    if (this->_input->getDigitalActionData(this->_startSwipingHandle).state)
    {
        if (this->_clickControl != this->_hoverControl) this->_clickControl = this->_hoverControl;
    }

    if (!this->_input->getDigitalActionData(this->_startSwipingHandle).state)
    {
        if (this->_clickControl != nullptr)
        {
            this->_clickedControls.push(this->_clickControl);
            this->_clickControl = nullptr;
        }
    }
}

void UI::render(int width, int height, float scale)
{
    nvgBeginFrame(this->vg, width, height, float(width) / float(height));

    for (auto control : this->_controls) control->render(this->vg, scale);

//#ifdef _WIN32
//    nvgSave(vg);

//    nvgTranslate(vg, this->currentMousePos().x, this->currentMousePos().y);
//    nvgScale(vg, scale, scale);
//    char icon[8];
//    nvgFontSize(vg, 48.0f);
//    nvgFontFace(vg, "fontawesome");
//    nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
//    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
//    if (this->_input->getDigitalActionData(this->_startSwipingHandle).state)
//        nvgText(vg, 0.0f, 0.0f, cpToUTF8(int(eFontAwesomeIcons::FA_HAND_GRAB_O), icon), NULL);
//    else
//        nvgText(vg, 0.0f, 0.0f, cpToUTF8(int(eFontAwesomeIcons::FA_HAND_STOP_O), icon), NULL);

//    nvgRestore(vg);
//#endif // _WIN32

    nvgEndFrame(this->vg);
}

void UI::addToGroup(GameModes mode, Control* control)
{
    if (this->_groups.find(mode) == this->_groups.end())
        this->_groups.insert(std::make_pair(mode, std::list<Control*>()));
    this->_groups[mode].insert(this->_groups[mode].end(), control);
}

void UI::removeFromGroup(GameModes mode, Control* control)
{
    if (this->_groups.find(mode) != this->_groups.end())
    {
        auto pos = std::find(this->_groups[mode].begin(), this->_groups[mode].end(), control);
        if (pos != this->_groups[mode].end()) this->_groups[mode].erase(pos);
    }
}

void UI::changeGameMode(GameModes mode)
{
    if (this->_groups.find(mode) != this->_groups.end())
        this->_controls = this->_groups[mode];
    else
        this->_controls = std::list<Control*>();

    this->_clickControl = nullptr;
    while (!this->_clickedControls.empty()) this->_clickedControls.pop();
    this->_hoverControl = nullptr;
}

void UI::setHoverControl(Control* control)
{
    this->_hoverControl = control;
}

const Control* UI::hoverControl() const
{
    return this->_hoverControl;
}

std::queue<Control*>& UI::clickedControls()
{
    return this->_clickedControls;
}

glm::vec2 UI::currentMousePos()
{
    auto data = this->_input->getAnalogActionData(this->_swipeHandle);
    return glm::vec2(data.x, data.y);
}





Control::Control(const std::string& id)
    : _id(id), _color(glm::vec4(255.0f, 255.0f, 255.0f, 255.0f)), _scale(1.0f), _fontFamily("sans-bold")
{ }

Control::~Control()
{ }

glm::vec2 Control::getEffectiveSize() const
{
    return this->size();
}

glm::vec2 Control::getEffectivePosition() const
{
    return this->position();
}

void Control::update(float elapsed)
{
    auto s = this->getEffectiveSize();
    auto p = this->getEffectivePosition();
    auto m = UI::Manager().currentMousePos();

    if (m.x >= p.x && m.x <= (p.x+s.x) && m.y <= (p.y+s.y) && m.y >= p.y)
    {
        UI::Manager().setHoverControl(this);
    }
    else if (UI::Manager().hoverControl() == this)
    {
        UI::Manager().setHoverControl(nullptr);
    }

    if (UI::Manager().hoverControl() == this)
    {
        this->_scale += 1.0f * elapsed;
        if (this->_scale > 1.1f) this->_scale = 1.1f;
    }
    else if (this->_scale > 1.0f)
    {
        this->_scale -= 1.0f * elapsed;
        if (this->_scale < 1.0f) this->_scale = 1.0f;
    }
}

const std::string& Control::id() const
{
    return this->_id;
}

void Control::setText(const std::string& text)
{
    this->_text = text;
}

const std::string& Control::text() const
{
    return this->_text;
}

void Control::setFontFamily(const std::string& font)
{
    this->_fontFamily = font;
}

const std::string& Control::fontFamily() const
{
    return this->_fontFamily;
}

void Control::setPosition(const glm::vec2& position)
{
    this->_position = position;
}

const glm::vec2& Control::position() const
{
    return this->_position;
}

void Control::setSize(const glm::vec2& size)
{
    this->_size = size;
}

const glm::vec2& Control::size() const
{
    return this->_size;
}

void Control::setPadding(const glm::vec2& padding)
{
    this->_padding = padding;
}

const glm::vec2& Control::padding() const
{
    return this->_padding;
}

void Control::setColor(const glm::vec4& color)
{
    this->_color = color;
}

const glm::vec4& Control::color() const
{
    return this->_color;
}

void Control::setBorderColor(const glm::vec4& color)
{
    this->_borderColor = color;
}

const glm::vec4& Control::borderColor() const
{
    return this->_borderColor;
}






Label::Label(const std::string &id)
    : Control(id)
{ }

Label::~Label()
{ }

void Label::render(NVGcontext* vg, float scale)
{
    nvgSave(vg);

    nvgTranslate(vg, this->_position.x, this->_position.y);
    nvgScale(vg, scale, scale);

    nvgFontSize(vg, this->size().y);
    nvgFontFace(vg, this->fontFamily().c_str());
    nvgFillColor(vg, nvgRGBA(this->color().r, this->color().g, this->color().b, this->color().a));
    nvgTextAlign(vg,NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(vg, 0.0f, 0.0f, this->_text.c_str(), NULL);

    nvgRestore(vg);
}






Button::Button(const std::string& id)
    : Control(id), _icon(eFontAwesomeIcons::FA_UNDO), _iconFontFamily("fontawesome")
{
    this->setSize(glm::vec2(60.0f, 60.0f));
}

Button::~Button()
{ }

void Button::onClick(std::function<void (const Button*)> func)
{
    this->_onClickfunc = func;
}

void Button::click() const
{
    if (this->_onClickfunc) this->_onClickfunc(this);
}

glm::vec2 Button::getEffectivePosition() const
{
    return this->position() - (this->size() / 2.0f);
}

void Button::setIconFontFamily(const std::string& font)
{
    this->_iconFontFamily = font;
}

const std::string& Button::iconFontFamily() const
{
    return this->_iconFontFamily;
}

void Button::render(NVGcontext* vg, float scale)
{
    nvgSave(vg);

    nvgTranslate(vg, this->_position.x, this->_position.y);
    nvgScale(vg, scale * this->_scale, scale * this->_scale);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, -(this->size().x / 2.0f), -(this->size().y / 2.0f), this->size().x, this->size().y, 4.0f);
    nvgFillColor(vg, nvgRGBA(68, 71, 78, 255));
    nvgFill(vg);

    char icon[8];
    nvgFontSize(vg, (this->size().x * 0.5f));
    nvgFontFace(vg, this->iconFontFamily().c_str());
    nvgFillColor(vg, nvgRGBA(this->color().r, this->color().g, this->color().b, this->color().a));
    nvgTextAlign(vg,NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(vg, 0.0f, 0.0f, cpToUTF8(int(this->_icon), icon), NULL);

    nvgScale(vg, 1.0f / this->_scale, 1.0f / this->_scale);
    nvgFontSize(vg, 32.0f);
    nvgFontFace(vg, this->fontFamily().c_str());
    nvgFillColor(vg, nvgRGBA(this->color().r, this->color().g, this->color().b, this->color().a));
    if (this->textAlignment() == eAlignments::Left)
    {
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgText(vg, this->size().x / 1.5f, 0.0f, this->_text.c_str(), NULL);
    }
    else if (this->textAlignment() == eAlignments::Right)
    {
        nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        nvgText(vg, -this->size().x / 1.5f, 0.0f, this->_text.c_str(), NULL);
    }
    else if (this->textAlignment() == eAlignments::Top)
    {
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(vg, 0.0f, -this->size().x / 1.2f, this->_text.c_str(), NULL);
    }
    else if (this->textAlignment() == eAlignments::Bottom)
    {
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(vg, 0.0f, this->size().x / 1.2f, this->_text.c_str(), NULL);
    }

    nvgRestore(vg);
}

void Button::setIcon(eFontAwesomeIcons icon)
{
    this->_icon = icon;
}

const eFontAwesomeIcons Button::icon() const
{
    return this->_icon;
}

void Button::setTextAlignment(eAlignments alignment)
{
    this->_textAlignment = alignment;
}

const eAlignments Button::textAlignment() const
{
    return this->_textAlignment;
}



PlayerButton::PlayerButton(const std::string &id, class Player* player)
    : Button(id), _player(player)
{ }

PlayerButton::~PlayerButton() { }

class Player* PlayerButton::player()
{
    return this->_player;
}

void PlayerButton::render(NVGcontext* vg, float scale)
{
    if (this->player()->_health <= 0.0f && this->icon() != eFontAwesomeIcons::FA_CLOSE)
    {
        this->setIcon(eFontAwesomeIcons::FA_CLOSE);
        this->setColor(glm::vec4(255, 0, 0, 255));
    }
    Button::render(vg, scale);

    nvgSave(vg);

    nvgTranslate(vg, this->_position.x, this->_position.y);
    nvgScale(vg, scale * this->_scale, scale * this->_scale);

    if (Player::Manager()._selectedPlayer == this->player())
    {
        nvgBeginPath(vg);
        nvgRoundedRect(vg, -(this->size().x / 2.0f), -(this->size().y / 2.0f), this->size().x, this->size().y, 4.0f);
        nvgStrokeColor(vg, nvgRGBA(255, 50, 50, 255));
        nvgStrokeWidth(vg, 2.0f);
        nvgStroke(vg);
    }

    nvgRestore(vg);
}



Panel::Panel(const std::string &id)
    : Control(id)
{ }

Panel::~Panel()
{ }

void Panel::render(NVGcontext* vg, float scale)
{
    nvgSave(vg);

    nvgTranslate(vg, this->position().x, this->position().y);
    nvgScale(vg, scale, scale);

    nvgBeginPath(vg);
    nvgRect(vg, 0.0f, 0.0f, this->size().x, this->size().y);
    nvgFillColor(vg, nvgRGBA(this->color().r, this->color().g, this->color().b, this->color().a));
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 0.0f, 0.0f, this->size().x, this->size().y);
    nvgStrokeColor(vg, nvgRGBA(this->borderColor().r, this->borderColor().g, this->borderColor().b, this->borderColor().a));
    nvgStrokeWidth(vg, 4.0f);
    nvgStroke(vg);

    nvgRestore(vg);
}
