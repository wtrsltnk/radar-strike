#ifndef INPUT_H
#define INPUT_H

#include <map>
#include <string>
#include "iinput.h"

enum class ActionStates
{
    Pressed,
    Released
};

enum class MouseButtons
{
    Left,
    Right,
    Middle,
    Button4,
    Button5,

    Count
};

enum class KeyboardKeys
{
    Return,
    Escape,
    Backspace,
    Tab,
    KP_Up,
    KP_Down,
    KP_Left,
    KP_Right,
    Character_Space,
    Character_A,
    Character_B,
    Character_C,
    Character_D,
    Character_E,
    Character_F,
    Character_G,
    Character_H,
    Character_I,
    Character_J,
    Character_K,
    Character_L,
    Character_M,
    Character_N,
    Character_O,
    Character_P,
    Character_Q,
    Character_R,
    Character_S,
    Character_T,
    Character_U,
    Character_V,
    Character_W,
    Character_X,
    Character_Y,
    Character_Z,

    Count
};

class Input : public IInput
{
    // Handles
    std::map<std::string, DigitalActionHandle> _digitalActionHandles;
    std::map<std::string, AnalogActionHandle> _analogActionHandles;

    // States
    DigitalActionState _digitalActionStates[MAX_DIGITAL_ACTIONS + 1];
    AnalogActionState _analogActionStates[MAX_ANALOG_ACTIONS + 1];

    // Mappings
    DigitalActionHandle _digitalMapping_OnKeyAction[int(KeyboardKeys::Count)];
    DigitalActionHandle _digitalMapping_OnMouseAction[int(MouseButtons::Count)];
    AnalogActionHandle _analogMapping_OnMouseMove;

public:
    Input();
    virtual ~Input();

    virtual void OnKeyAction(KeyboardKeys key, ActionStates state);
    virtual void OnMouseAction(MouseButtons button, ActionStates state);
    virtual void OnMouseMove(double x, double y);

    virtual DigitalActionHandle getDigitalActionHandle(const std::string& name) const;
    virtual DigitalActionState getDigitalActionData(DigitalActionHandle action) const;

    virtual AnalogActionHandle getAnalogActionHandle(const std::string& name) const;
    virtual AnalogActionState getAnalogActionData(AnalogActionHandle action) const;

};

#endif // INPUT_H
