#include "LogicInputCell.h"

LogicInputCell::LogicInputCell():
    LogicBaseCell(0, 1),
    mState(LogicState::LOW)
{}

void LogicInputCell::ToggleState()
{
    mState = ((mState == LogicState::HIGH) ? LogicState::LOW : LogicState::HIGH);

    NotifySuccessor(0, mState);
    emit StateChangedSignal();
}

LogicState LogicInputCell::GetOutputState(uint32_t pOutput) const
{
    Q_UNUSED(pOutput);
    return mState;
}

void LogicInputCell::OnWakeUp()
{
    mState = LogicState::LOW;
    emit StateChangedSignal();
}

void LogicInputCell::OnShutdown()
{
    mState = LogicState::LOW;
    emit StateChangedSignal();
}