#ifndef CORELOGIC_H
#define CORELOGIC_H

#include "View/View.h"
#include "Undo/UndoBaseType.h"
#include "Components/LogicWire.h"
#include "Components/ConPoint.h"
#include "Configuration.h"

#include <QGraphicsItem>
#include <deque>

class View;

class CoreLogic : public QObject
{
    Q_OBJECT
public:
    CoreLogic(View &pView);

    QGraphicsItem* GetItem(void);

    void EnterControlMode(ControlMode pMode);
    ControlMode GetControlMode(void);

    void SetComponentType(ComponentType pComponentType);
    ComponentType GetComponentType(void) const;

    void EnterAddControlMode(ComponentType pComponentType);

    void Undo(void);
    void Redo(void);

    void AddCurrentTypeComponent(QPointF pPosition);

    void SetPreviewWireStart(QPointF pStartPoint);

    /// \brief Draws preview wires to the current mouse position
    /// \param pCurrentPoint: The current mouse position
    void ShowPreviewWires(QPointF pCurrentPoint);

    /// \brief Adds wires from mPreviewWireStart to pEndPoint, with the first wire being
    /// added in mWireStartDirection direction, if two wires are added
    /// \param pEndPoint: The end point the last drawn wire should reach
    void AddWires(QPointF pEndPoint);

    void CopySelectedComponents(void);
    void DeleteSelectedComponents(void);

    void OnSelectedComponentsMoved(QPointF pOffset);

    bool IsSimulationRunning(void) const;

signals:
    void ControlModeChangedSignal(ControlMode pNewMode);
    void ComponentTypeChangedSignal(ComponentType pNewType);
    void MousePressedEventDefaultSignal(QMouseEvent &pEvent);

public slots:
    void OnLeftMouseButtonPressed(QPointF pMappedPos, QMouseEvent &pEvent);
    void OnConnectionTypeChanged(ConPoint* pConPoint, ConnectionType pPreviousType, ConnectionType pCurrentType);

protected:
    void ConnectToView(void);
    void AppendUndo(UndoBaseType* pUndoObject);
    void AppendToUndoQueue(UndoBaseType* pUndoObject, std::deque<UndoBaseType*> &pQueue);

    // Helper functions for wire processing

    std::vector<BaseComponent*> FilterForWires(const QList<QGraphicsItem*> &pComponents, WireDirection pDirection = WireDirection::UNSET) const;

    std::vector<BaseComponent*> GetCollidingComponents(QGraphicsItem* &pComponent);
    bool IsCollidingComponent(QGraphicsItem* pComponent) const;
    LogicWire* MergeWires(LogicWire* pNewWire, LogicWire* pLeftTopAdjacent, LogicWire* pRightBottomAdjacent) const;
    std::vector<BaseComponent*> DeleteContainedWires(LogicWire* pWire);
    LogicWire* GetAdjacentWire(QPointF pCheckPosition, WireDirection pDirection) const;
    void MergeWiresAfterMove(QList<QGraphicsItem*> &pComponents, std::vector<BaseComponent*> &pAddedWires, std::vector<BaseComponent*> &pDeletedWires);

    bool IsTCrossing(const LogicWire* pWire1, const LogicWire* pWire2) const;
    bool IsNoCrossingPoint(const ConPoint* pConPoint) const;
    bool IsXCrossingPoint(QPointF pPoint) const;

    template<typename T>
    bool IsComponentAtPosition(QPointF pPos);

protected:
    View &mView;

    ControlMode mControlMode = ControlMode::EDIT;

    ComponentType mComponentType = ComponentType::NONE;
    Direction mComponentDirection = components::DEFAULT_DIRECTION;
    uint8_t mComponentInputCount = components::gates::DEFAULT_INPUT_COUNT;

    std::deque<UndoBaseType*> mUndoQueue;
    std::deque<UndoBaseType*> mRedoQueue;

    QPointF mPreviewWireStart;
    WireDirection mWireStartDirection = WireDirection::HORIZONTAL;
    LogicWire mHorizontalPreviewWire;
    LogicWire mVerticalPreviewWire;
};

#endif // CORELOGIC_H
