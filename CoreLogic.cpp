#include "CoreLogic.h"

#include "Components/Gates/AndGate.h"
#include "Components/Gates/OrGate.h"
#include "Components/Gates/XorGate.h"
#include "Components/Gates/NotGate.h"
#include "Components/Inputs/LogicInput.h"
#include "Components/Inputs/LogicButton.h"
#include "Components/Inputs/LogicClock.h"
#include "Components/Outputs/LogicOutput.h"
#include "Components/TextLabel.h"
#include "Components/ComplexLogic/RsFlipFlop.h"
#include "Components/ComplexLogic/DFlipFlop.h"

#include "Undo/UndoAddType.h"
#include "Undo/UndoDeleteType.h"
#include "Undo/UndoMoveType.h"
#include "Undo/UndoConfigureType.h"

#include "HelperFunctions.h"

#include <QElapsedTimer>
#include <QCoreApplication>

CoreLogic::CoreLogic(View &pView):
    mView(pView),
    mHorizontalPreviewWire(this, WireDirection::HORIZONTAL, 0),
    mVerticalPreviewWire(this, WireDirection::VERTICAL, 0),
    mPropagationTimer(this),
    mProcessingTimer(this)
{
    ConnectToView();
    mView.Init();

    QObject::connect(&mPropagationTimer, &QTimer::timeout, this, &CoreLogic::OnPropagationTimeout);
    QObject::connect(&mProcessingTimer, &QTimer::timeout, this, &CoreLogic::OnProcessingTimeout);
}

void CoreLogic::ConnectToView()
{
    QObject::connect(this, &CoreLogic::ControlModeChangedSignal, &mView, &View::OnControlModeChanged);
    QObject::connect(this, &CoreLogic::ComponentTypeChangedSignal, &mView, &View::OnComponentTypeChanged);
}

void CoreLogic::EnterControlMode(ControlMode pMode)
{
    if (pMode == ControlMode::EDIT)
    {
        mView.Scene()->clearFocus();
    }

    if (pMode != mControlMode)
    {
        if (mControlMode == ControlMode::SIMULATION)
        {
            mControlMode = pMode;
            mPropagationTimer.stop();
            emit SimulationStopSignal();
        }

        mControlMode = pMode;
        emit ControlModeChangedSignal(pMode);

        if (pMode == ControlMode::ADD)
        {
            emit ComponentTypeChangedSignal(mComponentType);
        }

        if (pMode == ControlMode::SIMULATION)
        {
            StartSimulation();
        }
    }
}

void CoreLogic::StartSimulation()
{
    mView.SetGuiEnabled(false);
    StartProcessing();
    ParseWireGroups();
    CreateWireLogicCells();
    ConnectLogicCells();
    //qDebug() << "Found " << mWireGroups.size() << " groups";
    EndProcessing();
    mPropagationTimer.start(simulation::PROPAGATION_DELAY);
    emit SimulationStartSignal();
}

void CoreLogic::EnterAddControlMode(ComponentType pComponentType)
{
    EnterControlMode(ControlMode::ADD);
    SelectComponentType(pComponentType);
}

ComponentType CoreLogic::GetSelectedComponentType() const
{
    return mComponentType;
}

bool CoreLogic::IsSimulationRunning(void) const
{
    return (mControlMode == ControlMode::SIMULATION);
}

void CoreLogic::OnPropagationTimeout()
{
    emit SimulationAdvanceSignal();
}

bool CoreLogic::IsUndoQueueEmpty(void) const
{
    return mUndoQueue.empty();
}

bool CoreLogic::IsRedoQueueEmpty(void) const
{
    return mRedoQueue.empty();
}

void CoreLogic::SelectComponentType(ComponentType pComponentType)
{
    mComponentType = pComponentType;
    emit ComponentTypeChangedSignal(mComponentType);
}

IBaseComponent* CoreLogic::GetItem()
{
    IBaseComponent* item;

    switch(mComponentType)
    {
        case ComponentType::AND_GATE:
        {
            item = new AndGate(this, mComponentInputCount, mComponentDirection);
            break;
        }
        case ComponentType::OR_GATE:
        {
            item = new OrGate(this, mComponentInputCount, mComponentDirection);
            break;
        }
        case ComponentType::XOR_GATE:
        {
            item = new XorGate(this, mComponentInputCount, mComponentDirection);
            break;
        }
        case ComponentType::NOT_GATE:
        {
            item = new NotGate(this, mComponentDirection);
            break;
        }
        case ComponentType::INPUT:
        {
            item = new LogicInput(this);
            break;
        }
        case ComponentType::BUTTON:
        {
            item = new LogicButton(this);
            break;
        }
        case ComponentType::CLOCK:
        {
            item = new LogicClock(this, mComponentDirection);
            break;
        }
        case ComponentType::OUTPUT:
        {
            item = new LogicOutput(this);
            break;
        }
        case ComponentType::TEXT_LABEL:
        {
            item = new TextLabel(this);
            break;
        }
        case ComponentType::RS_FLIPFLOP:
        {
            item = new RsFlipFlop(this, mComponentDirection);
            break;
        }
        case ComponentType::D_FLIPFLOP:
        {
            item = new DFlipFlop(this, mComponentDirection);
            break;
        }
        default:
        {
            item = nullptr;
            break;
        }
    }

    return item;
}

ControlMode CoreLogic::GetControlMode()
{
    return mControlMode;
}

void CoreLogic::AddCurrentTypeComponent(QPointF pPosition)
{
    auto item = GetItem();
    Q_ASSERT(item);

    item->setPos(SnapToGrid(pPosition));

    if (GetCollidingComponents(item).empty())
    {
        mView.Scene()->clearFocus(); // Remove focus from components like labels that can be edited while in ADD mode
        mView.Scene()->addItem(item);

        auto addedComponents = std::vector<IBaseComponent*>{static_cast<IBaseComponent*>(item)};
        AppendUndo(new UndoAddType(addedComponents));
    }
    else
    {
        delete item;
    }
}

void CoreLogic::SetComponentInputCount(uint8_t pCount)
{
    mComponentInputCount = pCount;
}

void CoreLogic::SetComponentInputDirection(Direction pDirection)
{
    mComponentDirection = pDirection;
}

void CoreLogic::SetPreviewWireStart(QPointF pPoint)
{
    mPreviewWireStart = SnapToGrid(pPoint);

    mHorizontalPreviewWire.SetLength(0);
    mVerticalPreviewWire.SetLength(0);

    mView.Scene()->addItem(&mHorizontalPreviewWire);
    mView.Scene()->addItem(&mVerticalPreviewWire);
}

void CoreLogic::ShowPreviewWires(QPointF pCurrentPoint)
{
    QPointF snappedCurrentPoint = SnapToGrid(pCurrentPoint);

    // Set the start direction (which wire is drawn starting at the start position)
    if (mWireStartDirection == WireDirection::UNSET)
    {
        if (snappedCurrentPoint.x() != mPreviewWireStart.x())
        {
            mWireStartDirection = WireDirection::HORIZONTAL;
        }
        else if (snappedCurrentPoint.y() != mPreviewWireStart.y())
        {
            mWireStartDirection = WireDirection::VERTICAL;
        }
    }

    // Trigger a redraw of the area where the wires were before
    mHorizontalPreviewWire.setVisible(false);
    mVerticalPreviewWire.setVisible(false);

    mHorizontalPreviewWire.SetLength(std::abs(mPreviewWireStart.x() - snappedCurrentPoint.x()));
    mVerticalPreviewWire.SetLength(std::abs(mPreviewWireStart.y() - snappedCurrentPoint.y()));

    if (mWireStartDirection == WireDirection::HORIZONTAL)
    {
        mHorizontalPreviewWire.setPos(std::min(mPreviewWireStart.x(), snappedCurrentPoint.x()), mPreviewWireStart.y());
        mVerticalPreviewWire.setPos(snappedCurrentPoint.x(), std::min(mPreviewWireStart.y(), snappedCurrentPoint.y()));
    }
    else
    {
        mVerticalPreviewWire.setPos(mPreviewWireStart.x(), std::min(mPreviewWireStart.y(), snappedCurrentPoint.y()));
        mHorizontalPreviewWire.setPos(std::min(mPreviewWireStart.x(), snappedCurrentPoint.x()), snappedCurrentPoint.y());
    }

    mHorizontalPreviewWire.setVisible(true);
    mVerticalPreviewWire.setVisible(true);
}

void CoreLogic::AddWires(QPointF pEndPoint)
{
    mView.Scene()->removeItem(&mHorizontalPreviewWire);
    mView.Scene()->removeItem(&mVerticalPreviewWire);

    if (mWireStartDirection == WireDirection::UNSET)
    {
        return; // Return if no wire must be drawn anyways
    }

    QPointF snappedEndPoint = SnapToGrid(pEndPoint);
    std::vector<IBaseComponent*> addedComponents;
    std::vector<IBaseComponent*> deletedComponents;

    // Add horizontal wire
    if (mPreviewWireStart.x() != snappedEndPoint.x())
    {
        auto item = new LogicWire(this, WireDirection::HORIZONTAL, std::abs(mPreviewWireStart.x() - snappedEndPoint.x()));

        if (mWireStartDirection == WireDirection::HORIZONTAL)
        {
            item->setPos(std::min(mPreviewWireStart.x(), snappedEndPoint.x()), mPreviewWireStart.y());
        }
        else
        {
            item->setPos(std::min(mPreviewWireStart.x(), snappedEndPoint.x()), snappedEndPoint.y());
        }

        // Delete wires that are completely behind the new wire
        const auto containedWires = DeleteContainedWires(item);
        deletedComponents.insert(deletedComponents.end(), containedWires.begin(), containedWires.end());

        // Find wires left or right of the new wire (those may be partly behind the new wire)
        auto startAdjacent = GetAdjacentWire(QPointF(item->x() - 2, item->y()), WireDirection::HORIZONTAL);
        auto endAdjacent = GetAdjacentWire(QPointF(item->x() + item->GetLength() + 2, item->y()), WireDirection::HORIZONTAL);

        auto horizontalWire = MergeWires(item, startAdjacent, endAdjacent);

        delete item;

        if (startAdjacent == endAdjacent)
        {
            endAdjacent = nullptr;
        }

        if (startAdjacent)
        {
            deletedComponents.push_back(static_cast<IBaseComponent*>(startAdjacent));
            mView.Scene()->removeItem(startAdjacent);
        }

        if (endAdjacent)
        {
            deletedComponents.push_back(static_cast<IBaseComponent*>(endAdjacent));
            mView.Scene()->removeItem(endAdjacent);
        }

        mView.Scene()->addItem(horizontalWire);
        addedComponents.push_back(static_cast<IBaseComponent*>(horizontalWire));
    }

    // Add vertical wire
    if (mPreviewWireStart.y() != snappedEndPoint.y())
    {
        auto item = new LogicWire(this, WireDirection::VERTICAL, std::abs(mPreviewWireStart.y() - snappedEndPoint.y()));

        if (mWireStartDirection == WireDirection::VERTICAL)
        {
            item->setPos(mPreviewWireStart.x(), std::min(mPreviewWireStart.y(), snappedEndPoint.y()));
        }
        else
        {
            item->setPos(snappedEndPoint.x(), std::min(mPreviewWireStart.y(), snappedEndPoint.y()));
        }

        // Delete wires that are completely behind the new wire
        const auto containedWires = DeleteContainedWires(item);
        deletedComponents.insert(deletedComponents.end(), containedWires.begin(), containedWires.end());

        // Find wires above or below of the new wire (those may be partly behind the new wire)
        auto startAdjacent = GetAdjacentWire(QPointF(item->x(), item->y() - 2), WireDirection::VERTICAL);
        auto endAdjacent = GetAdjacentWire(QPointF(item->x(), item->y() + item->GetLength() + 2), WireDirection::VERTICAL);

        auto verticalWire = MergeWires(item, startAdjacent, endAdjacent);

        delete item;

        if (startAdjacent == endAdjacent)
        {
            endAdjacent = nullptr;
        }

        if (startAdjacent)
        {
            deletedComponents.push_back(static_cast<IBaseComponent*>(startAdjacent));
            mView.Scene()->removeItem(startAdjacent);
        }

        if (endAdjacent)
        {
            deletedComponents.push_back(static_cast<IBaseComponent*>(endAdjacent));
            mView.Scene()->removeItem(endAdjacent);
        }

        mView.Scene()->addItem(verticalWire);
        addedComponents.push_back(static_cast<IBaseComponent*>(verticalWire));
    }

    std::vector<IBaseComponent*> addedConPoints;

    for (const auto& wire : addedComponents)
    {
        for (const auto& collidingComp : mView.Scene()->collidingItems(wire, Qt::IntersectsItemShape))
        {
            if (dynamic_cast<LogicWire*>(collidingComp) != nullptr && IsTCrossing(static_cast<LogicWire*>(wire), static_cast<LogicWire*>(collidingComp)))
            {
                QPointF conPointPos;
                if (static_cast<LogicWire*>(wire)->GetDirection() == WireDirection::HORIZONTAL)
                {
                    conPointPos.setX(collidingComp->x());
                    conPointPos.setY(wire->y());
                }
                else
                {
                    conPointPos.setX(wire->x());
                    conPointPos.setY(collidingComp->y());
                }

                if (!IsComponentAtPosition<ConPoint>(conPointPos))
                {
                    auto item = new ConPoint(this);
                    item->setPos(conPointPos);
                    addedConPoints.push_back(item);
                    mView.Scene()->addItem(item);
                }
            }
        }
    }

    addedComponents.insert(addedComponents.end(), addedConPoints.begin(), addedConPoints.end());
    AppendUndo(new UndoAddType(addedComponents, deletedComponents));
    mWireStartDirection = WireDirection::UNSET;
}

template<typename T>
bool CoreLogic::IsComponentAtPosition(QPointF pPos)
{
    for (const auto& comp : mView.Scene()->items(pPos, Qt::IntersectsItemShape))
    {
        if (dynamic_cast<T*>(comp) != nullptr)
        {
            return true;
        }
    }
    return false;
}

void CoreLogic::MergeWiresAfterMove(std::vector<LogicWire*> &pWires, std::vector<IBaseComponent*> &pAddedWires, std::vector<IBaseComponent*> &pDeletedWires)
{
    for (auto &w : pWires)
    {
        const auto containedWires = DeleteContainedWires(w);
        pDeletedWires.insert(pDeletedWires.end(), containedWires.begin(), containedWires.end());

        LogicWire* startAdjacent = nullptr;
        LogicWire* endAdjacent = nullptr;

        if (w->GetDirection() == WireDirection::HORIZONTAL)
        {
            startAdjacent = GetAdjacentWire(QPointF(w->x() - 4, w->y()), WireDirection::HORIZONTAL);
            endAdjacent = GetAdjacentWire(QPointF(w->x() + w->GetLength() + 4, w->y()), WireDirection::HORIZONTAL);

            auto horizontalWire = MergeWires(w, startAdjacent, endAdjacent);
            horizontalWire->setSelected(w->isSelected());

            if (startAdjacent == endAdjacent)
            {
                endAdjacent = nullptr;
            }

            mView.Scene()->addItem(horizontalWire);
            pAddedWires.push_back(static_cast<IBaseComponent*>(horizontalWire));
        }
        else
        {
            startAdjacent = GetAdjacentWire(QPointF(w->x(), w->y() - 4), WireDirection::VERTICAL);
            endAdjacent = GetAdjacentWire(QPointF(w->x(), w->y() + w->GetLength() + 4), WireDirection::VERTICAL);

            auto verticalWire = MergeWires(w, startAdjacent, endAdjacent);
            verticalWire->setSelected(w->isSelected());

            if (startAdjacent == endAdjacent)
            {
                endAdjacent = nullptr;
            }

            mView.Scene()->addItem(verticalWire);
            pAddedWires.push_back(static_cast<IBaseComponent*>(verticalWire));
        }

        pDeletedWires.push_back(w);
        mView.Scene()->removeItem(w);

        if (startAdjacent && std::find(pAddedWires.begin(), pAddedWires.end(), startAdjacent) == pAddedWires.end())
        {
            pDeletedWires.push_back(static_cast<IBaseComponent*>(startAdjacent));
            mView.Scene()->removeItem(startAdjacent);
        }

        if (endAdjacent && std::find(pAddedWires.begin(), pAddedWires.end(), endAdjacent) == pAddedWires.end())
        {
            pDeletedWires.push_back(static_cast<IBaseComponent*>(endAdjacent));
            mView.Scene()->removeItem(endAdjacent);
        }
    }
}

std::vector<IBaseComponent*> CoreLogic::DeleteContainedWires(LogicWire* pWire)
{
    std::vector<IBaseComponent*> deletedComponents;

    QRectF collisionRect;
    if (pWire->GetDirection() == WireDirection::HORIZONTAL)
    {
        collisionRect = QRectF(pWire->x() - 2, pWire->y() - components::wires::BOUNDING_RECT_SIZE / 2 - 2,
                               pWire->GetLength() + 4, components::wires::BOUNDING_RECT_SIZE + 4);
    }
    else
    {
        collisionRect = QRectF(pWire->x() - components::wires::BOUNDING_RECT_SIZE / 2 - 2, pWire->y() - 2,
                               components::wires::BOUNDING_RECT_SIZE + 4, pWire->GetLength() + 4);
    }

    const auto&& containedComponents = mView.Scene()->items(collisionRect, Qt::ContainsItemShape, Qt::DescendingOrder);

    for (const auto &wire : containedComponents)
    {
        if (dynamic_cast<LogicWire*>(wire) != nullptr && static_cast<LogicWire*>(wire)->GetDirection() == pWire->GetDirection() && wire != pWire)
        {
            deletedComponents.push_back(static_cast<IBaseComponent*>(wire));
            mView.Scene()->removeItem(wire);
        }
    }

    return deletedComponents;
}

LogicWire* CoreLogic::GetAdjacentWire(QPointF pCheckPosition, WireDirection pDirection) const
{
    const auto startAdjacentComponents = mView.Scene()->items(pCheckPosition.x(), pCheckPosition.y(), 1, 1, Qt::IntersectsItemShape, Qt::DescendingOrder);
    const auto startAdjacentWires = FilterForWires(startAdjacentComponents, pDirection);

    if (startAdjacentWires.size() > 0)
    {
        return static_cast<LogicWire*>(startAdjacentWires.at(0));
    }

    return nullptr;
}

// Remember that using (dynamic_cast<LogicWire*>(comp) != nullptr) directly is more efficient than iterating over filtered components
std::vector<IBaseComponent*> CoreLogic::FilterForWires(const QList<QGraphicsItem*> &pComponents, WireDirection pDirection) const
{
    std::vector<IBaseComponent*> wires;

    for (auto &comp : pComponents)
    {
        if (dynamic_cast<LogicWire*>(comp) != nullptr
                && (static_cast<LogicWire*>(comp)->GetDirection() == pDirection || pDirection == WireDirection::UNSET))
        {
            wires.push_back(static_cast<IBaseComponent*>(comp));
        }
    }

    return wires;
}

std::vector<IBaseComponent*> CoreLogic::GetCollidingComponents(IBaseComponent* pComponent) const
{
    std::vector<IBaseComponent*> collidingComponents;

    for (auto &comp : mView.Scene()->collidingItems(pComponent))
    {
        if (IsCollidingComponent(comp))
        {
            // comp must be IBaseComponent at this point
            collidingComponents.push_back(static_cast<IBaseComponent*>(comp));
        }
    }

    return collidingComponents;
}

bool CoreLogic::IsCollidingComponent(QGraphicsItem* pComponent) const
{
    return (dynamic_cast<IBaseComponent*>(pComponent) != nullptr
            && dynamic_cast<LogicWire*>(pComponent) == nullptr
            && dynamic_cast<ConPoint*>(pComponent) == nullptr
            && dynamic_cast<TextLabel*>(pComponent) == nullptr);
}

bool CoreLogic::IsTCrossing(const LogicWire* pWire1, const LogicWire* pWire2) const
{
    const LogicWire* a;
    const LogicWire* b;

    if (pWire1->GetDirection() == WireDirection::VERTICAL && pWire2->GetDirection() == WireDirection::HORIZONTAL)
    {
        a = pWire1;
        b = pWire2;
    }
    else if (pWire1->GetDirection() == WireDirection::HORIZONTAL && pWire2->GetDirection() == WireDirection::VERTICAL)
    {
        a = pWire2;
        b = pWire1;
    }
    else
    {
        return false;
    }

    return ((a->y() < b->y() && a->x() == b->x() && a->y() + a->GetLength() > b->y())
        || (a->y() < b->y() && a->x() == b->x() + b->GetLength() && a->y() + a->GetLength() > b->y())
        || (a->x() > b->x() && a->y() + a->GetLength() == b->y() && a->x() < b->x() + b->GetLength())
        || (a->x() > b->x() && a->y() == b->y() && a->x() < b->x() + b->GetLength()));
}

bool CoreLogic::IsNoCrossingPoint(const ConPoint* pConPoint) const
{
    const auto&& components = mView.Scene()->items(pConPoint->pos(), Qt::IntersectsItemBoundingRect);

    if (components.size() <= 2)
    {
        // Including the ConPoint at the position, there can be at max the ConPoint and one wire
        return true;
    }
    else
    {
        bool foundOne = false;
        bool firstGoesTrough = false;
        for (const auto& comp : components)
        {
            if (dynamic_cast<LogicWire*>(comp) != nullptr)
            {
                if (!foundOne)
                {
                    foundOne = true; // Found a crossing wire (either ends in pConPoint or doesn't)
                    firstGoesTrough = !static_cast<LogicWire*>(comp)->StartsOrEndsIn(pConPoint->pos()); // True, if this wire doesn't end in pConPoint
                }
                else if ((foundOne && firstGoesTrough) || (foundOne && !static_cast<LogicWire*>(comp)->StartsOrEndsIn(pConPoint->pos())))
                {
                    // T-Crossing wire found (first or second one) and two wires total, this is no L or I crossing
                    return false;
                }
            }
        }
        return true;
    }
}

bool CoreLogic::IsXCrossingPoint(QPointF pPoint) const
{
    const auto& wires = FilterForWires(mView.Scene()->items(pPoint, Qt::IntersectsItemBoundingRect));

    if (wires.size() <= 1)
    {
        return false;
    }
    else
    {
        for (const auto& wire : wires)
        {
            if (static_cast<LogicWire*>(wire)->StartsOrEndsIn(pPoint))
            {
                // L-Crossing type wire found, this is no X crossing
                return false;
            }
        }
        return true;
    }
}

LogicWire* CoreLogic::MergeWires(LogicWire* pNewWire, LogicWire* pStartAdjacent, LogicWire* pEndAdjacent) const
{
    QPointF newStart(pNewWire->pos());

    if (pNewWire->GetDirection() == WireDirection::HORIZONTAL)
    {
        QPointF newEnd(pNewWire->x() + pNewWire->GetLength(), pNewWire->y());

        if (pStartAdjacent && pStartAdjacent->GetDirection() == pNewWire->GetDirection())
        {
            Q_ASSERT(pNewWire->y() == pStartAdjacent->y());
            newStart = QPointF(pStartAdjacent->x(), pNewWire->y());
        }
        if (pEndAdjacent && pEndAdjacent->GetDirection() == pNewWire->GetDirection())
        {
            Q_ASSERT(pNewWire->y() == pEndAdjacent->y());
            newEnd = QPoint(pEndAdjacent->x() + pEndAdjacent->GetLength(), pNewWire->y());
        }

        auto newWire = new LogicWire(this, WireDirection::HORIZONTAL, newEnd.x() - newStart.x());
        newWire->setPos(newStart);
        return newWire;
    }
    else
    {
        QPointF newEnd(pNewWire->x(), pNewWire->y() + pNewWire->GetLength());

        if (pStartAdjacent && pStartAdjacent->GetDirection() == pNewWire->GetDirection())
        {
            Q_ASSERT(pNewWire->x() == pStartAdjacent->x());
            newStart = QPointF(pNewWire->x(), pStartAdjacent->y());
        }
        if (pEndAdjacent && pEndAdjacent->GetDirection() == pNewWire->GetDirection())
        {
            Q_ASSERT(pNewWire->x() == pEndAdjacent->x());
            newEnd = QPoint(pNewWire->x(), pEndAdjacent->y() + pEndAdjacent->GetLength());
        }

        auto newWire = new LogicWire(this, WireDirection::VERTICAL, newEnd.y() - newStart.y());
        newWire->setPos(newStart);
        return newWire;
    }
}

void CoreLogic::ParseWireGroups(void)
{
    mWireGroups.clear();
    mWireMap.clear();

    for (auto& comp : mView.Scene()->items())
    {
        // If the component is a wire that is not yet part of a group
        if (dynamic_cast<LogicWire*>(comp) && mWireMap.find(static_cast<LogicWire*>(comp)) == mWireMap.end())
        {
            mWireGroups.push_back(std::vector<IBaseComponent*>());
            ExploreGroup(static_cast<LogicWire*>(comp), mWireGroups.size() - 1);
        }
        ProcessingHeartbeat();
    }
}

void CoreLogic::ExploreGroup(LogicWire* pWire, int32_t pGroupIndex)
{
    mWireMap[pWire] = pGroupIndex;
    mWireGroups[pGroupIndex].push_back(pWire); // Note: pWire must not be part of group pGroupIndex before the call to ExploreGroup

    for (auto& coll : mView.Scene()->collidingItems(pWire, Qt::IntersectsItemShape)) // Item shape is sufficient for wire collision
    {
        if (dynamic_cast<LogicWire*>(coll) != nullptr && mWireMap.find(static_cast<LogicWire*>(coll)) == mWireMap.end())
        {
            auto conPoint = GetConPointAtPosition(GetWireCollisionPoint(pWire, static_cast<LogicWire*>(coll)), ConnectionType::FULL);
            if (conPoint != nullptr)
            {
                mWireGroups[pGroupIndex].push_back(conPoint);
            }
            if (conPoint != nullptr || IsLCrossing(pWire, static_cast<LogicWire*>(coll)))
            {
                ExploreGroup(static_cast<LogicWire*>(coll), pGroupIndex); // Recursive call
            }
        }
        ProcessingHeartbeat();
    }
}

QPointF CoreLogic::GetWireCollisionPoint(const LogicWire* pWireA, const LogicWire* pWireB) const
{
    // Assuming the wires do collide
    if (pWireA->GetDirection() == WireDirection::HORIZONTAL && pWireB->GetDirection() == WireDirection::VERTICAL)
    {
        return QPointF(pWireB->x(), pWireA->y());
    }
    else if (pWireA->GetDirection() == WireDirection::VERTICAL && pWireB->GetDirection() == WireDirection::HORIZONTAL)
    {
        return QPointF(pWireA->x(), pWireB->y());
    }
    else
    {
        Q_ASSERT(false);
    }
}

bool CoreLogic::IsLCrossing(LogicWire* pWireA, LogicWire* pWireB) const
{
    const LogicWire* a;
    const LogicWire* b;

    if (pWireA->GetDirection() == WireDirection::VERTICAL && pWireB->GetDirection() == WireDirection::HORIZONTAL)
    {
        a = pWireB;
        b = pWireA;
    }
    else if (pWireA->GetDirection() == WireDirection::HORIZONTAL && pWireB->GetDirection() == WireDirection::VERTICAL)
    {
        a = pWireA;
        b = pWireB;
    }
    else
    {
        return false;
    }

    return ((a->y() == b->y() && a->x() == b->x()) || (a->y() == b->y() && a->x() + a->GetLength() == b->x())
        || (a->x() == b->x() && b->y() + b->GetLength() == a->y()) || (a->x() + a->GetLength() == b->x() && a->y() == b->y() + b->GetLength()));
}

ConPoint* CoreLogic::GetConPointAtPosition(QPointF pPos, ConnectionType pType) const
{
    for (const auto& comp : mView.Scene()->items(pPos, Qt::IntersectsItemShape))
    {
        if (dynamic_cast<ConPoint*>(comp) != nullptr)
        {
            if(static_cast<ConPoint*>(comp)->GetConnectionType() == pType)
            {
                return static_cast<ConPoint*>(comp);
            }
        }
    }

    return nullptr;
}

void CoreLogic::CreateWireLogicCells()
{
    mLogicWireCells.clear();

    for (const auto& group : mWireGroups)
    {
        auto logicCell = std::make_shared<LogicWireCell>(this);
        mLogicWireCells.emplace_back(logicCell);
        for (auto& comp : group)
        {
            if (dynamic_cast<LogicWire*>(comp) != nullptr)
            {
                static_cast<LogicWire*>(comp)->SetLogicCell(logicCell);
            }
            else if (dynamic_cast<ConPoint*>(comp) != nullptr)
            {
                static_cast<ConPoint*>(comp)->SetLogicCell(logicCell);
            }
            ProcessingHeartbeat();
        }
    }
}

void CoreLogic::ConnectLogicCells()
{
    for (auto& comp : mView.Scene()->items())
    {
        if (dynamic_cast<IBaseComponent*>(comp) == nullptr || dynamic_cast<LogicWire*>(comp) != nullptr
                || (dynamic_cast<ConPoint*>(comp) != nullptr && static_cast<ConPoint*>(comp)->GetConnectionType() == ConnectionType::FULL))
        {
            continue;
        }

        auto compBase = static_cast<IBaseComponent*>(comp);

        for (auto& coll : mView.Scene()->collidingItems(comp, Qt::IntersectsItemBoundingRect))
        {
            if (dynamic_cast<ConPoint*>(coll) != nullptr && static_cast<ConPoint*>(coll)->GetConnectionType() == ConnectionType::FULL)
            {
                continue; // ignore ConPoints, they are handled during wire grouping
            }
            else if (dynamic_cast<LogicWire*>(coll) != nullptr)
            {
                // Component <-> Wire connection
                auto wire = static_cast<LogicWire*>(coll);
                for (size_t out = 0; out < compBase->GetOutConnectorCount(); out++)
                {
                    if (wire->contains(wire->mapFromScene(compBase->pos() + compBase->GetOutConnectors()[out].pos)))
                    {
                        std::static_pointer_cast<LogicWireCell>(wire->GetLogicCell())->AddInputSlot();
                        compBase->GetLogicCell()->ConnectOutput(wire->GetLogicCell(), std::static_pointer_cast<LogicWireCell>(wire->GetLogicCell())->GetInputSize() - 1, out);
                        //qDebug() << "Connected comp output " << out << " to wire";
                    }
                }

                for (size_t in = 0; in < compBase->GetInConnectorCount(); in++)
                {
                    if (wire->contains(wire->mapFromScene(compBase->pos() + compBase->GetInConnectors()[in].pos)))
                    {
                        std::static_pointer_cast<LogicWireCell>(wire->GetLogicCell())->AppendOutput(compBase->GetLogicCell(), in);
                        //qDebug() << "Connected wire to comp, input " << in;
                    }
                }
            }
            ProcessingHeartbeat();
        }
    }
}

void CoreLogic::StartProcessing()
{
    mProcessingTimer.start(gui::PROCESSING_OVERLAY_TIMEOUT);
    mIsProcessing = true;
}

void CoreLogic::ProcessingHeartbeat()
{
    // User input during processing will be handled but ignored
    QCoreApplication::processEvents();
}

void CoreLogic::OnProcessingTimeout()
{
    mView.FadeInProcessingOverlay();
}

void CoreLogic::EndProcessing()
{
    mProcessingTimer.stop();
    mView.FadeOutProcessingOverlay();
    mIsProcessing = false;
}

bool CoreLogic::IsProcessing()
{
    return mIsProcessing;
}

void CoreLogic::OnSelectedComponentsMoved(QPointF pOffset)
{   
    qDebug() << "Move started";

    mView.SetGuiEnabled(false);
    StartProcessing();

    QElapsedTimer total;
    total.start();
    if (pOffset.manhattanLength() <= 0)
    {
        // No effective movement
        EndProcessing();
        mView.PrepareGuiForEditing();
        return;
    }

    std::vector<LogicWire*> affectedWires;
    std::vector<IBaseComponent*> affectedComponents;

    for (const auto& comp : mView.Scene()->selectedItems())
    {
        if (dynamic_cast<IBaseComponent*>(comp) != nullptr)
        {
            if (dynamic_cast<LogicWire*>(comp) != nullptr)
            {
                affectedWires.push_back(static_cast<LogicWire*>(comp));
            }

            affectedComponents.push_back(static_cast<IBaseComponent*>(comp));
        }

        ProcessingHeartbeat();
    }

    QElapsedTimer merging;
    merging.start();
    std::vector<IBaseComponent*> movedComponents;
    std::vector<IBaseComponent*> addedComponents;
    std::vector<IBaseComponent*> deletedComponents;

    MergeWiresAfterMove(affectedWires, addedComponents, deletedComponents); // Ca. 25% of total cost

    // Insert merged wires to recognize T-crossings
    affectedComponents.insert(affectedComponents.end(), addedComponents.begin(), addedComponents.end());
    // In theory, we should remove deletedComponents from movedComponents here, but that would be costly and
    // should not make any difference because old wires behind the merged ones cannot generate new ConPoints

    qDebug() << "Merging wires and inserting into affected components took " << merging.elapsed() << "ms";

    QElapsedTimer conpoints;
    conpoints.start();

    for (const auto& comp : affectedComponents) // Ca. 75% of total cost
    {                   
        if (!ManageConPointsOneStep(comp, pOffset, movedComponents, addedComponents, deletedComponents))
        {
            // Collision, abort
            for (const auto& comp : affectedComponents) // Revert moving
            {
                comp->moveBy(-pOffset.x(), -pOffset.y());
            }
            for (const auto& comp : addedComponents) // Revert adding
            {
                delete comp;
            }
            for (const auto& comp : deletedComponents) // Revert deleting
            {
                mView.Scene()->addItem(comp);
            }
            mView.Scene()->clearSelection();
            EndProcessing();
            mView.PrepareGuiForEditing();
            return;
        }

        ProcessingHeartbeat();
    }

    qDebug() << "Managing ConPoints took " << conpoints.elapsed() << "ms";

    mView.Scene()->clearSelection();

    if (movedComponents.size() > 0)
    {
        AppendUndo(new UndoMoveType(movedComponents, addedComponents, deletedComponents, pOffset));
    }
    qDebug() << "Moving took " << total.elapsed() << "ms total";

    EndProcessing();
    mView.PrepareGuiForEditing();
}

bool CoreLogic::ManageConPointsOneStep(IBaseComponent* comp, QPointF& pOffset, std::vector<IBaseComponent*>& movedComponents,
                                       std::vector<IBaseComponent*>& addedComponents, std::vector<IBaseComponent*>& deletedComponents)
{
    // Delete all invalid ConPoints at the original position colliding with the selection
    QRectF oldCollisionRect(comp->pos() + comp->boundingRect().topLeft() - pOffset,
                                       comp->pos() + comp->boundingRect().bottomRight() - pOffset);

    const auto&& abandonedComponents = mView.Scene()->items(oldCollisionRect, Qt::IntersectsItemShape);

    for (const auto& collidingComp : abandonedComponents)
    {
        if (dynamic_cast<ConPoint*>(collidingComp) != nullptr && !collidingComp->isSelected() && IsNoCrossingPoint(static_cast<ConPoint*>(collidingComp)))
        {
            mView.Scene()->removeItem(collidingComp);
            deletedComponents.push_back(static_cast<IBaseComponent*>(collidingComp));
        }
        ProcessingHeartbeat();
    }

    // Delete all ConPoints of the moved components that are not valid anymore
    if (dynamic_cast<ConPoint*>(comp) != nullptr && IsNoCrossingPoint(static_cast<ConPoint*>(comp)))
    {
        mView.Scene()->removeItem(comp);
        deletedComponents.push_back(comp);
    }

    // Add ConPoints to all T Crossings
    if (dynamic_cast<LogicWire*>(comp) != nullptr) // Costly
    {
        const auto&& collidingComponents = mView.Scene()->collidingItems(comp, Qt::IntersectsItemShape);

        for (const auto& collidingComp : collidingComponents)
        {
            if (dynamic_cast<LogicWire*>(collidingComp) != nullptr && IsTCrossing(static_cast<LogicWire*>(comp), static_cast<LogicWire*>(collidingComp)))
            {
                QPointF conPointPos;
                if (static_cast<LogicWire*>(comp)->GetDirection() == WireDirection::HORIZONTAL)
                {
                    conPointPos = QPointF(collidingComp->x(), comp->y());
                }
                else
                {
                    conPointPos = QPointF(comp->x(), collidingComp->y());
                }

                if (!IsComponentAtPosition<ConPoint>(conPointPos)) // Might be costly
                {
                    auto item = new ConPoint(this);
                    item->setPos(conPointPos);
                    addedComponents.push_back(item);
                    mView.Scene()->addItem(item);
                }
            }

            ProcessingHeartbeat();
        }
    }

    if (IsCollidingComponent(comp) && !GetCollidingComponents(comp).empty())
    {
        return false;
    }

    movedComponents.push_back(comp);
    return true;
}

void CoreLogic::OnLeftMouseButtonPressedWithoutCtrl(QPointF pMappedPos, QMouseEvent &pEvent)
{
    auto snappedPos = SnapToGrid(pMappedPos);

    if (mControlMode != ControlMode::SIMULATION)
    {
        // Add ConPoint on X crossing
        if (mControlMode == ControlMode::EDIT
                && mView.Scene()->selectedItems().empty() // Scene must be empty (select of clicked item did not yet happen)
                && dynamic_cast<LogicWire*>(mView.Scene()->itemAt(pMappedPos, QTransform())) != nullptr // Wire is clicked (not crossing below other component)
                && IsXCrossingPoint(snappedPos) // There is an X-crossing at that position
                && !IsComponentAtPosition<ConPoint>(snappedPos)) // There is no ConPoint at that position yet
        {
            // Create a new ConPoint (removing will be handled by OnConnectionTypeChanged)
            auto item = new ConPoint(this);
            item->setPos(snappedPos);
            std::vector<IBaseComponent*> addedComponents{item};
            mView.Scene()->addItem(item);
            AppendUndo(new UndoAddType(addedComponents));
            return;
        }

        // Invert in/output connectors
        if (mControlMode == ControlMode::EDIT
                && mView.Scene()->selectedItems().empty())
        {
            for (const auto& item : mView.Scene()->items(pMappedPos, Qt::IntersectsItemBoundingRect))
            {
                if (dynamic_cast<AbstractGate*>(item) != nullptr || dynamic_cast<AbstractComplexLogic*>(item) != nullptr || dynamic_cast<LogicClock*>(item) != nullptr)
                {
                    const auto&& connector = static_cast<IBaseComponent*>(item)->InvertConnectorByPoint(pMappedPos);
                    if (connector != nullptr)
                    {
                        auto data = std::make_shared<Undo::ConnectorInversionChangedData>(static_cast<IBaseComponent*>(item), connector);
                        AppendUndo(new UndoConfigureType(data));
                        return;
                    }
                }
            }
        }

        // Add component at the current position
        if (mControlMode == ControlMode::ADD)
        {
            AddCurrentTypeComponent(snappedPos);
            return;
        }

        // Start the preview wire at the current position
        if (mControlMode == ControlMode::WIRE)
        {
            SetPreviewWireStart(snappedPos);
            return;
        }
    }

    emit MousePressedEventDefaultSignal(pEvent);
}

void CoreLogic::OnConnectionTypeChanged(ConPoint* pConPoint, ConnectionType pPreviousType, ConnectionType pCurrentType)
{
    Q_ASSERT(pConPoint);

    if (IsXCrossingPoint(pConPoint->pos()) && pPreviousType == ConnectionType::DIODE_X)
    {
        pConPoint->setSelected(false);
        pConPoint->SetConnectionType(pPreviousType); // Restore old connection type in case delete is undone
        mView.Scene()->removeItem(pConPoint);

        auto deleted = std::vector<IBaseComponent*>{pConPoint};
        AppendUndo(new UndoDeleteType(deleted));
    }
    else
    {
        auto data = std::make_shared<Undo::ConnectionTypeChangedData>(pConPoint, pPreviousType, pCurrentType);
        AppendUndo(new UndoConfigureType(data));
    }
}

void CoreLogic::OnTextLabelContentChanged(TextLabel* pTextLabel, QString pPreviousText, QString pCurrentText)
{
    Q_ASSERT(pTextLabel);

    auto data = std::make_shared<Undo::TextLabelContentChangedData>(pTextLabel, pPreviousText, pCurrentText);
    AppendUndo(new UndoConfigureType(data));
}

void CoreLogic::CopySelectedComponents()
{
    QList<QGraphicsItem*> componentsToCopy = mView.Scene()->selectedItems();
    std::vector<IBaseComponent*> addedComponents{};

    mView.Scene()->clearSelection();

    for (auto& orig : componentsToCopy)
    {
        // Create a copy of the original component
        IBaseComponent* copy = static_cast<IBaseComponent*>(orig)->CloneBaseComponent(this);
        Q_ASSERT(copy);

        // Paste the copied component one grid cell below and to the right
        copy->setPos(SnapToGrid(orig->pos() + QPointF(canvas::GRID_SIZE, canvas::GRID_SIZE)));
        copy->setSelected(true);
        copy->ResetZValue();
        copy->setZValue(copy->zValue() + 100); // Bring copied components to front
        mView.Scene()->addItem(copy);
        addedComponents.push_back(copy);
    }
    if (addedComponents.size() > 0)
    {
        AppendUndo(new UndoAddType(addedComponents));
    }
}

void CoreLogic::DeleteSelectedComponents()
{
    QList<QGraphicsItem*> componentsToDelete = mView.Scene()->selectedItems();
    std::vector<IBaseComponent*> deletedComponents{};
    for (auto& comp : componentsToDelete)
    {
        // Do not allow deleting of ConPoints on T crossings
        if (dynamic_cast<ConPoint*>(comp) == nullptr || IsXCrossingPoint(comp->pos()))
        {
            mView.Scene()->removeItem(comp);
            deletedComponents.push_back(static_cast<IBaseComponent*>(comp));
        }
    }

    // Delete all colliding ConPoints that are not over a crossing anymore
    for (auto& comp : FilterForWires(componentsToDelete))
    {
        for (const auto& collidingComp : mView.Scene()->collidingItems(comp))
        {
            if (dynamic_cast<ConPoint*>(collidingComp) != nullptr && IsNoCrossingPoint(static_cast<ConPoint*>(collidingComp)))
            {
                mView.Scene()->removeItem(collidingComp);
                deletedComponents.push_back(static_cast<IBaseComponent*>(collidingComp));
            }
        }
    }

    if (deletedComponents.size() > 0)
    {
        AppendUndo(new UndoDeleteType(deletedComponents));
    }
}

void CoreLogic::AppendUndo(UndoBaseType* pUndoObject)
{
    AppendToUndoQueue(pUndoObject, mUndoQueue);
    mRedoQueue.clear();

    mView.SetUndoRedoButtonsEnableState();
}

void CoreLogic::AppendToUndoQueue(UndoBaseType* pUndoObject, std::deque<UndoBaseType*> &pQueue)
{
    pQueue.push_back(pUndoObject);
    if (pQueue.size() > MAX_UNDO_STACK_SIZE)
    {
        delete pQueue.front();
        pQueue.pop_front();
    }
}

void CoreLogic::Undo()
{
    if (mUndoQueue.size() > 0)
    {
        UndoBaseType* undoObject = mUndoQueue.back();
        mUndoQueue.pop_back();
        Q_ASSERT(undoObject);

        switch (undoObject->Type())
        {
            case Undo::Type::ADD:
            {
                for (const auto& comp : static_cast<UndoAddType*>(undoObject)->AddedComponents())
                {
                    Q_ASSERT(comp);
                    mView.Scene()->removeItem(comp);
                }
                for (const auto& comp : static_cast<UndoAddType*>(undoObject)->DeletedComponents())
                {
                    Q_ASSERT(comp);
                    mView.Scene()->addItem(comp);
                }
                AppendToUndoQueue(undoObject, mRedoQueue);
                break;
            }
            case Undo::Type::DEL:
            {
                for (const auto& comp : static_cast<UndoDeleteType*>(undoObject)->Components())
                {
                    Q_ASSERT(comp);
                    mView.Scene()->addItem(comp);
                }
                AppendToUndoQueue(undoObject, mRedoQueue);
                break;
            }
            case Undo::Type::MOVE:
            {
                const auto undoMoveObject = static_cast<UndoMoveType*>(undoObject);
                for (const auto& comp : static_cast<UndoMoveType*>(undoObject)->DeletedComponents())
                {
                    Q_ASSERT(comp);
                    mView.Scene()->addItem(comp);
                }
                for (const auto& comp : static_cast<UndoMoveType*>(undoObject)->AddedComponents())
                {
                    Q_ASSERT(comp);
                    mView.Scene()->removeItem(comp);
                }
                for (const auto& comp : undoMoveObject->MovedComponents())
                {
                    Q_ASSERT(comp);
                    comp->moveBy(-undoMoveObject->Offset().x(), -undoMoveObject->Offset().y());
                }
                AppendToUndoQueue(undoObject, mRedoQueue);
                break;
            }
            case Undo::Type::CONFIGURE:
            {
                const auto undoConfigureObject = static_cast<UndoConfigureType*>(undoObject);
                switch (undoConfigureObject->Data()->Type())
                {
                    case Undo::ConfigType::CONNECTION_TYPE:
                    {
                        auto data = std::static_pointer_cast<Undo::ConnectionTypeChangedData>(undoConfigureObject->Data());
                        Q_ASSERT(data->conPoint);
                        data->conPoint->SetConnectionType(data->previousType);
                        AppendToUndoQueue(undoObject, mRedoQueue);
                        break;
                    }
                case Undo::ConfigType::TEXTLABEL_CONTENT:
                {
                    auto data = std::static_pointer_cast<Undo::TextLabelContentChangedData>(undoConfigureObject->Data());
                    Q_ASSERT(data->textLabel);
                    data->textLabel->SetTextContent(data->previousText);
                    AppendToUndoQueue(undoObject, mRedoQueue);
                    break;
                }
                case Undo::ConfigType::CONNECTOR_INVERSION:
                {
                    auto data = std::static_pointer_cast<Undo::ConnectorInversionChangedData>(undoConfigureObject->Data());
                    Q_ASSERT(data->component);
                    Q_ASSERT(data->logicConnector);
                    data->component->InvertConnectorByPoint(data->component->pos() + data->logicConnector->pos);
                    AppendToUndoQueue(undoObject, mRedoQueue);
                    break;
                }
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
    mView.Scene()->clearSelection();
    mView.SetUndoRedoButtonsEnableState();
}

void CoreLogic::Redo()
{
    if (mRedoQueue.size() > 0)
    {
        UndoBaseType* redoObject = mRedoQueue.back();
        mRedoQueue.pop_back();
        Q_ASSERT(redoObject);

        switch (redoObject->Type())
        {
            case Undo::Type::ADD:
            {
                for (const auto& comp : static_cast<UndoAddType*>(redoObject)->AddedComponents())
                {
                    Q_ASSERT(comp);
                    mView.Scene()->addItem(comp);
                }
                for (const auto& comp : static_cast<UndoAddType*>(redoObject)->DeletedComponents())
                {
                    Q_ASSERT(comp);
                    mView.Scene()->removeItem(comp);
                }
                AppendToUndoQueue(redoObject, mUndoQueue);
                break;
            }
            case Undo::Type::DEL:
            {
                for (const auto& comp : static_cast<UndoDeleteType*>(redoObject)->Components())
                {
                    Q_ASSERT(comp);
                    mView.Scene()->removeItem(comp);
                }
                AppendToUndoQueue(redoObject, mUndoQueue);
                break;
            }
            case Undo::Type::MOVE:
            {
                const auto redoMoveObject = static_cast<UndoMoveType*>(redoObject);
                for (const auto& comp : redoMoveObject->MovedComponents())
                {
                    Q_ASSERT(comp);
                    comp->moveBy(redoMoveObject->Offset().x(), redoMoveObject->Offset().y());
                }
                for (const auto& comp : static_cast<UndoMoveType*>(redoObject)->AddedComponents())
                {
                    Q_ASSERT(comp);
                    mView.Scene()->addItem(comp);
                }
                for (const auto& comp : static_cast<UndoMoveType*>(redoObject)->DeletedComponents())
                {
                    Q_ASSERT(comp);
                    mView.Scene()->removeItem(comp);
                }
                AppendToUndoQueue(redoObject, mUndoQueue);
                break;
            }
            case Undo::Type::CONFIGURE:
            {
                const auto undoConfigureObject = static_cast<UndoConfigureType*>(redoObject);
                switch (undoConfigureObject->Data()->Type())
                {
                    case Undo::ConfigType::CONNECTION_TYPE:
                    {
                        auto data = std::static_pointer_cast<Undo::ConnectionTypeChangedData>(undoConfigureObject->Data());
                        Q_ASSERT(data->conPoint);
                        data->conPoint->SetConnectionType(data->currentType);
                        AppendToUndoQueue(redoObject, mUndoQueue);
                        break;
                    }
                    case Undo::ConfigType::TEXTLABEL_CONTENT:
                    {
                        auto data = std::static_pointer_cast<Undo::TextLabelContentChangedData>(undoConfigureObject->Data());
                        Q_ASSERT(data->textLabel);
                        data->textLabel->SetTextContent(data->currentText);
                        AppendToUndoQueue(redoObject, mUndoQueue);
                        break;
                    }
                    case Undo::ConfigType::CONNECTOR_INVERSION:
                    {
                        auto data = std::static_pointer_cast<Undo::ConnectorInversionChangedData>(undoConfigureObject->Data());
                        Q_ASSERT(data->component);
                        Q_ASSERT(data->logicConnector);
                        data->component->InvertConnectorByPoint(data->component->pos() + data->logicConnector->pos);
                        AppendToUndoQueue(redoObject, mUndoQueue);
                        break;
                    }
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
    mView.Scene()->clearSelection();
    mView.SetUndoRedoButtonsEnableState();
}
