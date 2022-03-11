#include "LogicButtonCell.h"
#include "Configuration.h"

LogicButtonCell::LogicButtonCell():
    LogicBaseCell(0, 1),
    mState(LogicState::LOW),
    mButtonTimer(this)
{
    mButtonTimer.setInterval(components::inputs::BUTTON_TOGGLE_INTERVAL);
    mButtonTimer.setSingleShot(true);

    QObject::connect(&mButtonTimer, &QTimer::timeout, this, &LogicButtonCell::ButtonTimeout);
}

void LogicButtonCell::ButtonClick()
{
    if (mState != LogicState::HIGH)
    {
        mState = LogicState::HIGH;
        NotifySuccessor(0, mState);
        emit StateChangedSignal();
    }
    mButtonTimer.start();
}

void LogicButtonCell::ButtonTimeout()
{
    mState = LogicState::LOW;
    NotifySuccessor(0, mState);
    emit StateChangedSignal();
}

LogicState LogicButtonCell::GetOutputState(uint32_t pOutput) const
{
    Q_UNUSED(pOutput);
    return mState;
}

void LogicButtonCell::OnWakeUp()
{
    mState = LogicState::LOW;
    emit StateChangedSignal();
}

void LogicButtonCell::OnShutdown()
{
    mState = LogicState::LOW;
    emit StateChangedSignal();
}