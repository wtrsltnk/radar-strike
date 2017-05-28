#ifndef UI_H
#define UI_H

#include <map>
#include <list>
#include <queue>
#include <string>
#include <functional>
#include <glm/glm.hpp>
#include "gamemodes.h"
#include "../iinput.h"
#include "../font-icons.h"

#include "nanovg.h"

class Control
{
    std::string _id;
protected:
    std::string _text;
    std::string _fontFamily;
    glm::vec2 _position;
    glm::vec2 _size;
    glm::vec2 _padding;
    glm::vec4 _color;
    glm::vec4 _borderColor;
    float _scale;

public:
    explicit Control(const std::string& id);
    virtual ~Control();

    virtual glm::vec2 getEffectiveSize() const;
    virtual glm::vec2 getEffectivePosition() const;

    virtual void update(float elapsed);
    virtual void render(NVGcontext* vg, float scale = 1.0f) = 0;

    const std::string& id() const;

    void setText(const std::string& text);
    const std::string& text() const;

    void setFontFamily(const std::string& font);
    const std::string& fontFamily() const;

    void setPosition(const glm::vec2& position);
    const glm::vec2& position() const;

    void setSize(const glm::vec2& size);
    const glm::vec2& size() const;

    void setPadding(const glm::vec2& padding);
    const glm::vec2& padding() const;

    void setColor(const glm::vec4& color);
    const glm::vec4& color() const;

    void setBorderColor(const glm::vec4& color);
    const glm::vec4& borderColor() const;
};

class Label : public Control
{
public:
    explicit Label(const std::string& id);
    virtual ~Label();

    virtual void render(NVGcontext* vg, float scale = 1.0f);

};

enum class eAlignments
{
    Top,
    Bottom,
    Left,
    Right
};

class Button : public Control
{
    std::function<void (const Button*)> _onClickfunc;
    eFontAwesomeIcons _icon;
    std::string _iconFontFamily;
    eAlignments _textAlignment;

public:
    explicit Button(const std::string& id);
    virtual ~Button();

    virtual glm::vec2 getEffectivePosition() const;

    virtual void render(NVGcontext* vg, float scale = 1.0f);

    void onClick(std::function<void (const Button*)> func);
    void click() const;

    void setIconFontFamily(const std::string& font);
    const std::string& iconFontFamily() const;

    void setIcon(eFontAwesomeIcons icon);
    const eFontAwesomeIcons icon() const;

    void setTextAlignment(eAlignments alignment);
    const eAlignments textAlignment() const;
};

class PlayerButton : public Button
{
    class Player* _player;
public:
    explicit PlayerButton(const std::string& id, class Player* player);
    virtual ~PlayerButton();

    virtual void render(NVGcontext* vg, float scale);

    class Player* player();
};

class Panel : public Control
{
public:
    explicit Panel(const std::string& id);
    virtual ~Panel();

    virtual void render(NVGcontext* vg, float scale = 1.0f);

};

class UI
{
    const IInput* _input;
    AnalogActionHandle _swipeHandle;
    DigitalActionHandle _startSwipingHandle;

    std::map<GameModes, std::list<Control*> > _groups;
    std::list<Control*> _controls;
    Control* _hoverControl;
    Control* _clickControl;
    std::queue<Control*> _clickedControls;

    NVGcontext* vg;

    static UI* _instance;
    UI();
public:
    static UI& Manager();
    virtual ~UI();

    void init(const IInput* input, NVGcontext* vg);
    virtual void update(float elapsed);
    virtual void render(int width, int height, float scale = 1.0f);

    void addToGroup(GameModes mode, Control* control);
    void removeFromGroup(GameModes mode, Control* control);
    void changeGameMode(GameModes mode);

    void setHoverControl(Control* control);
    const Control* hoverControl() const;
    std::queue<Control*>& clickedControls();

    glm::vec2 currentMousePos();
};

#endif // UI_H
