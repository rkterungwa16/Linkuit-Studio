#include "View.h"
#include "HelperFunctions.h"
#include "Components/TextLabel.h"

#include <QtWidgets>

void GraphicsView::wheelEvent(QWheelEvent *pEvent)
{
    Q_ASSERT(pEvent);
    if (pEvent->modifiers() & Qt::ControlModifier)
    {
        if (pEvent->angleDelta().y() > 0)
        {
            mView.ZoomIn(canvas::ZOOM_SPEED);
        }
        else
        {
            mView.ZoomOut(canvas::ZOOM_SPEED);
        }
        pEvent->accept();
    }
}

void GraphicsView::mousePressEvent(QMouseEvent *pEvent)
{
    Q_ASSERT(pEvent);
    if (pEvent->button() == Qt::LeftButton)
    {
        mIsLeftMousePressed = true;
        if (pEvent->modifiers() & Qt::ControlModifier)
        {
            // Start panning
            mPanStart = pEvent->pos();
            pEvent->accept();
        }
        else
        {
            emit LeftMouseButtonPressedWithoutCtrlEvent(mapToScene(pEvent->pos()), *pEvent);
        }
        return;
    }

    QGraphicsView::mousePressEvent(pEvent);
}

void GraphicsView::OnMousePressedEventDefault(QMouseEvent &pEvent)
{
    QGraphicsView::mousePressEvent(&pEvent);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    Q_ASSERT(pEvent);
    if (pEvent->button() == Qt::LeftButton)
    {
        mIsLeftMousePressed = false;

        if (!mCoreLogic.IsSimulationRunning())
        {
            // Snap all potentially moved components to grid
            for (auto& comp : scene()->selectedItems())
            {
                comp->setPos(SnapToGrid(comp->pos()));
            }
        }

        if (pEvent->modifiers() & Qt::ControlModifier)
        {
            pEvent->accept();
            return;
        }

        // Add the new wires at the current position
        if (mCoreLogic.GetControlMode() == ControlMode::WIRE)
        {
            mCoreLogic.AddWires(mapToScene(pEvent->pos()));
            return;
        }
    }

    QGraphicsView::mouseReleaseEvent(pEvent);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *pEvent)
{
    Q_ASSERT(pEvent);
    if (mIsLeftMousePressed)
    {
        mCoreLogic.ShowPreviewWires(mapToScene(pEvent->pos()));
    }

    if (mIsLeftMousePressed && pEvent->modifiers() & Qt::ControlModifier)
    {
        // Pan the scene
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (pEvent->position().x() - mPanStart.x()));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (pEvent->position().y() - mPanStart.y()));

        mPanStart = pEvent->pos();

        pEvent->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(pEvent);
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent *pEvent)
{
    // Prevent interaction while not in edit mode
    Q_UNUSED(pEvent);
    return;
}

void GraphicsView::keyPressEvent(QKeyEvent *pEvent)
{
    Q_ASSERT(pEvent);

    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            // Entering EDIT mode is also enabled during label editing
            mCoreLogic.EnterControlMode(ControlMode::EDIT);
            break;
        }
        case Qt::Key_Delete:
        {
            if (!mCoreLogic.IsSimulationRunning())
            {
                mCoreLogic.DeleteSelectedComponents();
            }
            break;
        }
        default:
        {
            break;
        }
    }

    QList<QGraphicsItem*>&& selected = mView.Scene()->selectedItems();

    if (selected.size() != 1 || dynamic_cast<TextLabel*>(selected[0]) == nullptr)
    {
        // Key actions that are disabled during label editing

        switch (pEvent->key())
        {
            case Qt::Key_Return:
            {
                if (mCoreLogic.IsSimulationRunning())
                {
                    mCoreLogic.EnterControlMode(ControlMode::EDIT);
                }
                else
                {
                    mCoreLogic.EnterControlMode(ControlMode::SIMULATION);
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }

    QGraphicsView::keyPressEvent(pEvent);
}
