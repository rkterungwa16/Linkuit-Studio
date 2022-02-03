#include "RectComponent.h"

RectComponent::RectComponent()
{
    setPos(0, 0);
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
}

void RectComponent::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget)
{
    Q_UNUSED(pWidget);

    const double levelOfDetail = pOption->levelOfDetailFromTransform(pPainter->worldTransform());

    QPen pen(pOption->state & QStyle::State_Selected ? components::SELECTED_BORDER_COLOR : components::BORDER_COLOR,
             components::BORDER_WIDTH, Qt::SolidLine, Qt::RoundCap);
    pPainter->setPen(pen);
    pPainter->setBrush(QBrush(components::FILL_COLOR));

    pPainter->drawRect(0, 0, mWidth, mHeight);
    PaintSpecifics(pPainter, levelOfDetail);
}

QRectF RectComponent::boundingRect() const
{
    return QRectF(0, 0, mWidth, mHeight);
}

QPainterPath RectComponent::shape() const
{
    QPainterPath path;
    path.addRect(0, 0, mWidth, mHeight);
    return path;
}
