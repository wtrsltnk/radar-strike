#ifndef IINPUT_H
#define IINPUT_H

#include <string>

#define MAX_ANALOG_ACTIONS 16

#define MAX_DIGITAL_ACTIONS 128

struct DigitalActionState
{
    bool state;
    bool active;
};

enum class AnalogActionSourceTypes
{
    AbsoluteMouse,
    RelativeMouse
};

struct AnalogActionState
{
    AnalogActionSourceTypes source;
    float x, y;
    bool active;
};

typedef unsigned int ActionHandle;
typedef ActionHandle DigitalActionHandle;
typedef ActionHandle AnalogActionHandle;

class IInput
{
public:
    virtual ~IInput();

    virtual DigitalActionHandle getDigitalActionHandle(const std::string& name) const = 0;
    virtual DigitalActionState getDigitalActionData(DigitalActionHandle action) const = 0;

    virtual AnalogActionHandle getAnalogActionHandle(const std::string& name) const = 0;
    virtual AnalogActionState getAnalogActionData(AnalogActionHandle action) const = 0;
};

#endif // IINPUT_H
