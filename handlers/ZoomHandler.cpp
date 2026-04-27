#include "ZoomHandler.h"

#include <QGraphicsView>
#include <QWheelEvent>
ZoomHandler::ZoomHandler(int priority, QObject *parent)
    : AbstractInteractionHandler(priority, parent)
    , m_zoomInFactor(1.15)
    , m_minScale(0.05)
    , m_maxScale(20.0)
    , m_currentScale(1.0)
    , m_anchorUnderMouse(true)
{
}

bool ZoomHandler::handleWheel(QGraphicsView *view, QWheelEvent *event)
{
    // 获取当前实际缩放比例
    const qreal currentTransformScale = view->transform().m11();

    // 根据滚轮方向确定缩放因子
    qreal factor;
    if (event->angleDelta().y() > 0) {
        factor = m_zoomInFactor;
    } else {
        factor = 1.0 / m_zoomInFactor;
    }

    // 计算新的缩放比例并进行范围限制
    const qreal newScale = currentTransformScale * factor;
    if (newScale < m_minScale || newScale > m_maxScale) {
        return true;
    }

    // 设置缩放锚点
    if (m_anchorUnderMouse) {
        view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    } else {
        view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    }

    view->scale(factor, factor);
    m_currentScale = view->transform().m11();

    return true;
}

QString ZoomHandler::handlerName() const
{
    return QStringLiteral("ZoomHandler");
}

void ZoomHandler::resetZoom(QGraphicsView *view)
{
    view->resetTransform();
    m_currentScale = 1.0;
}

void ZoomHandler::zoomToFit(QGraphicsView *view)
{
    if (!view->scene()) {
        return;
    }

    const QRectF itemsBound = view->scene()->itemsBoundingRect();
    if (itemsBound.isEmpty()) {
        return;
    }

    view->fitInView(itemsBound, Qt::KeepAspectRatio);
    m_currentScale = view->transform().m11();
}

qreal ZoomHandler::currentScale() const
{
    return m_currentScale;
}

void ZoomHandler::setZoomInFactor(qreal factor)
{
    if (factor > 1.0) {
        m_zoomInFactor = factor;
    }
}

void ZoomHandler::setMinScale(qreal scale)
{
    if (scale > 0) {
        m_minScale = scale;
    }
}

void ZoomHandler::setMaxScale(qreal scale)
{
    if (scale > m_minScale) {
        m_maxScale = scale;
    }
}

void ZoomHandler::setAnchorUnderMouse(bool enabled)
{
    m_anchorUnderMouse = enabled;
}
