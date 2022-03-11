#include "AbstractComplexLogic.h"
#include "Configuration.h"

AbstractComplexLogic::AbstractComplexLogic(const CoreLogic* pCoreLogic, std::shared_ptr<LogicBaseCell> pLogicCell, uint8_t pInputCount, uint8_t pOutputCount, Direction pDirection):
    IBaseComponent(pCoreLogic, pLogicCell),
    mInputCount(pInputCount),
    mOutputCount(pOutputCount),
    mDirection(pDirection),
    mInputInverted(pInputCount, false),
    mOutputInverted(pOutputCount, false)
{
    setZValue(components::zvalues::GATE);

    if (mDirection == Direction::RIGHT || mDirection == Direction::LEFT)
    {
        mWidth = components::gates::GRID_WIDTH * canvas::GRID_SIZE;
        mHeight = std::max(mInputCount, mOutputCount) * canvas::GRID_SIZE * 2;
    }
    else
    {
        mWidth = std::max(mInputCount, mOutputCount) * canvas::GRID_SIZE * 2;
        mHeight = components::gates::GRID_WIDTH * canvas::GRID_SIZE;
    }
}

void AbstractComplexLogic::ResetZValue()
{
    setZValue(components::zvalues::GATE);
}

void AbstractComplexLogic::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pItem, QWidget *pWidget)
{
    Q_UNUSED(pWidget);
    const double levelOfDetail = pItem->levelOfDetailFromTransform(pPainter->worldTransform());

    // Draw connectors and invertion circles
    {
        QPen pen(pItem->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::wires::WIRE_LOW_COLOR,
                 components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        pPainter->setPen(pen);
        if (levelOfDetail >= components::CONNECTORS_MIN_LOD)
        {
            // Draw connectors
            switch (mDirection)
            {
                case Direction::RIGHT:
                {
                    for (size_t i = 0; i < mInputCount; i++)
                    {
                        if (mLogicCell->GetInputState(i) == LogicState::HIGH)
                        {
                            pPainter->setPen(QPen(components::wires::WIRE_HIGH_COLOR, components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        }
                        else
                        {
                            pPainter->setPen(QPen(pItem->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::wires::WIRE_LOW_COLOR,
                                                  components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        }
                        pPainter->drawLine(-8, 2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE, 0, 2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE);
                    }
                    for (size_t i = 0; i < mOutputCount; i++)
                    {
                        if (mLogicCell->GetOutputState(i) == LogicState::HIGH)
                        {
                            pPainter->setPen(QPen(components::wires::WIRE_HIGH_COLOR, components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        }
                        else
                        {
                            pPainter->setPen(QPen(pItem->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::wires::WIRE_LOW_COLOR,
                                                  components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        }
                        pPainter->drawLine(mWidth, 2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE, mWidth + 8, 2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE);
                    }
                    break;
                }
#warning add output connectors for other directions
                case Direction::DOWN:
                {
                    for (size_t i = 0; i < mInputCount; i++)
                    {
                        if (mLogicCell->GetInputState(i) == LogicState::HIGH)
                        {
                            pPainter->setPen(QPen(components::wires::WIRE_HIGH_COLOR, components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        }
                        else
                        {
                            pPainter->setPen(QPen(pItem->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::wires::WIRE_LOW_COLOR,
                                                  components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        }
                        pPainter->drawLine(2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE, -8, 2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE, 0);
                    }
                    if (mLogicCell->GetOutputState() == LogicState::HIGH)
                    {
                        pPainter->setPen(QPen(components::wires::WIRE_HIGH_COLOR, components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    }
                    else
                    {
                        pPainter->setPen(QPen(pItem->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::wires::WIRE_LOW_COLOR,
                                              components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    }
                    pPainter->drawLine(mWidth / 2, mHeight, mWidth / 2, mHeight + 8);
                    break;
                }
                case Direction::LEFT:
                {
                    for (size_t i = 0; i < mInputCount; i++)
                    {
                        if (mLogicCell->GetInputState(i) == LogicState::HIGH)
                        {
                            pPainter->setPen(QPen(components::wires::WIRE_HIGH_COLOR, components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        }
                        else
                        {
                            pPainter->setPen(QPen(pItem->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::wires::WIRE_LOW_COLOR,
                                                  components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        }
                        pPainter->drawLine(mWidth, 2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE, mWidth + 8, 2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE);
                    }
                    if (mLogicCell->GetOutputState() == LogicState::HIGH)
                    {
                        pPainter->setPen(QPen(components::wires::WIRE_HIGH_COLOR, components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    }
                    else
                    {
                        pPainter->setPen(QPen(pItem->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::wires::WIRE_LOW_COLOR,
                                              components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    }
                    pPainter->drawLine(-8, mHeight / 2, 0, mHeight / 2);
                    break;
                }
                case Direction::UP:
                {
                    for (size_t i = 0; i < mInputCount; i++)
                    {
                        if (mLogicCell->GetInputState(i) == LogicState::HIGH)
                        {
                            pPainter->setPen(QPen(components::wires::WIRE_HIGH_COLOR, components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        }
                        else
                        {
                            pPainter->setPen(QPen(pItem->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::wires::WIRE_LOW_COLOR,
                                                  components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        }
                        pPainter->drawLine(2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE, mHeight, 2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE, mHeight + 8);
                    }
                    if (mLogicCell->GetOutputState() == LogicState::HIGH)
                    {
                        pPainter->setPen(QPen(components::wires::WIRE_HIGH_COLOR, components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    }
                    else
                    {
                        pPainter->setPen(QPen(pItem->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::wires::WIRE_LOW_COLOR,
                                              components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    }
                    pPainter->drawLine(mWidth / 2, -8, mWidth / 2, 0);
                    break;
                }
                default:
                {
                    Q_ASSERT(false);
                    break;
                }
            }

#warning set pen and brush for every output circle and every direction
            if (mLogicCell->GetOutputState() == LogicState::HIGH)
            {
                pPainter->setPen(QPen(components::wires::WIRE_HIGH_COLOR, components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                pPainter->setBrush(components::wires::WIRE_HIGH_COLOR);
            }
            else
            {
                pPainter->setPen(QPen(pItem->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::wires::WIRE_LOW_COLOR,
                                      components::wires::WIRE_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                pPainter->setBrush(pItem->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::wires::WIRE_LOW_COLOR);
            }

            // Draw invertion circles
            switch (mDirection)
            {
                case Direction::RIGHT:
                {
                    for (size_t i = 0; i < mInputCount; i++)
                    {
                        if (mInputInverted.at(i))
                        {
#warning outdated ellipse parameter
                            pPainter->drawEllipse(-10, 2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE - 5, 10, 10);
                        }
                    }
                    for (size_t i = 0; i < mOutputCount; i++)
                    {
                        if (mOutputInverted.at(i))
                        {
                            pPainter->drawEllipse(mWidth + 1, mHeight / 2 - 4, 8, 8);
                        }
                    }
                    break;
                }
                case Direction::DOWN:
                {
                    for (size_t i = 0; i < mInputCount; i++)
                    {
                        if (mInputInverted.at(i))
                        {
                            pPainter->drawEllipse(2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE - 5, -10, 10, 10);
                        }
                    }
                    /*if (mOutputInverted)
                    {
                        pPainter->drawEllipse(mWidth / 2 - 5, mHeight, 10, 10);
                    }*/
                    break;
                }
                case Direction::LEFT:
                {
                    for (size_t i = 0; i < mInputCount; i++)
                    {
                        if (mInputInverted.at(i))
                        {
                            pPainter->drawEllipse(mWidth, 2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE - 5, 10, 10);
                        }
                    }
                    /*if (mOutputInverted)
                    {
                        pPainter->drawEllipse(-10, mHeight / 2 - 5, 10, 10);
                    }*/
                    break;
                }
                case Direction::UP:
                {
                    for (size_t i = 0; i < mInputCount; i++)
                    {
                        if (mInputInverted.at(i))
                        {
                            pPainter->drawEllipse(2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE - 5, mHeight, 10, 10);
                        }
                    }
                    /*if (mOutputInverted)
                    {
                        pPainter->drawEllipse(mWidth / 2 - 5, -10, 10, 10);
                    }*/
                    break;
                }
                default:
                {
                    Q_ASSERT(false);
                    break;
                }
            }
        }
    }

    // Draw gate body
    {
        QPen pen(pItem->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::FILL_COLOR,
                 components::BORDER_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        pPainter->setPen(pen);
        pPainter->setBrush(QBrush(components::FILL_COLOR));

        /*if (levelOfDetail >= components::ROUNDED_CORNERS_MIN_LOD)
        {
            pPainter->drawRoundedRect(0, 0, mWidth, mHeight, 0, 0);
        }
        else
        {
            pPainter->drawRect(0, 0, mWidth, mHeight);
        }*/
        pPainter->drawRect(0, 0, mWidth, mHeight);
    }

    // Draw description text
    if (levelOfDetail >= components::DESCRIPTION_TEXT_MIN_LOD)
    {
        pPainter->setPen(components::complex_logic::FONT_COLOR);
        pPainter->setFont(components::complex_logic::FONT);
        pPainter->drawText(boundingRect(), mComponentText, Qt::AlignHCenter | Qt::AlignVCenter);

        pPainter->setPen(QColor(225, 225, 225));
        pPainter->setFont(components::complex_logic::CONNECTOR_FONT);
        switch (mDirection)
        {
            case Direction::RIGHT:
            {
                for (size_t i = 0; i < mInputCount; i++)
                {
                    pPainter->drawText(QRect(2, 2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE - 6, mWidth - 4, 10), mInputLabels[i], Qt::AlignLeft | Qt::AlignVCenter);
                }
                for (size_t i = 0; i < mOutputCount; i++)
                {
                    pPainter->drawText(QRect(2, 2 * canvas::GRID_SIZE * i + canvas::GRID_SIZE - 6, mWidth - 4, 10), mOutputLabels[i], Qt::AlignRight | Qt::AlignVCenter);
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

QRectF AbstractComplexLogic::boundingRect() const
{
    if (mDirection == Direction::RIGHT || mDirection == Direction::LEFT)
    {
        return QRectF(-13, -3, mWidth + 26, mHeight + 6);
    }
    else
    {
        return QRectF(-3, -13, mWidth + 6, mHeight + 26);
    }
}

QPainterPath AbstractComplexLogic::shape() const
{
    QPainterPath path;
    path.addRect(0, 0, mWidth, mHeight);
    return path;
}