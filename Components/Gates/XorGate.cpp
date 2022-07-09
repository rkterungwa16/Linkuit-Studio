#include "XorGate.h"
#include "CoreLogic.h"
#include "LogicCells/LogicXorGateCell.h"

XorGate::XorGate(const CoreLogic* pCoreLogic, uint8_t pInputCount, Direction pDirection):
    AbstractGate(pCoreLogic, std::make_shared<LogicXorGateCell>(pInputCount), pInputCount, pDirection)
{
    mComponentText = components::gates::XOR_TEXT;
}

XorGate::XorGate(const XorGate& pObj, const CoreLogic* pCoreLogic):
    XorGate(pCoreLogic, pObj.mInputCount, pObj.mDirection)
{
    mComponentText = pObj.mComponentText;
    mWidth = pObj.mWidth;
    mHeight = pObj.mHeight;
    mLogicCell->SetInputInversions(pObj.mLogicCell->GetInputInversions());
    mLogicCell->SetOutputInversions(pObj.mLogicCell->GetOutputInversions());
};

XorGate::XorGate(const CoreLogic* pCoreLogic, const QJsonObject& pJson):
    XorGate(pCoreLogic, pJson["inputs"].toInt(), static_cast<Direction>(pJson["dir"].toInt()))
{
    setPos(SnapToGrid(QPointF(pJson["x"].toInt(), pJson["y"].toInt())));

    std::vector<bool> ininv;
    auto ininvArray = pJson["ininv"].toArray();
    foreach (auto inv, ininvArray)
    {
        ininv.push_back(inv.toBool());
    }
    GetLogicCell()->SetInputInversions(ininv);
    GetLogicCell()->SetOutputInversions(std::vector<bool>{pJson["outinv"].toBool()});
}


IBaseComponent* XorGate::CloneBaseComponent(const CoreLogic* pCoreLogic) const
{
    return new XorGate(*this, pCoreLogic);
}

QJsonObject XorGate::GetJson() const
{
    QJsonObject json;

#warning use type string lookup table
    json["type"] = "XOR_GATE";
    json["x"] = x();
    json["y"] = y();
    json["dir"] = static_cast<int32_t>(mDirection);
    json["inputs"] = mInputCount;

    {
        QJsonArray ininv;

        foreach(const auto inv, mLogicCell->GetInputInversions())
        {
            ininv.append(inv);
        }

        json["ininv"] = ininv;

        json["outinv"] = QJsonValue(mLogicCell->GetOutputInversions()[0]);
    }

    return json;
}
