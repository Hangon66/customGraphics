#include "ZoomHandler.h"
#include "../view/CustomGraphicsView.h"

#include <QGraphicsView>
#include <QWheelEvent>
#include <QDebug>
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
    // 钳位到 [m_minScale, m_maxScale]，保证可以精确到达边界值
    qreal clampedScale = qBound(m_minScale, newScale, m_maxScale);
    if (qFuzzyCompare(clampedScale, currentTransformScale)) {
        return true; // 已经在边界，无需操作
    }
    qreal actualFactor = clampedScale / currentTransformScale;

    // 设置缩放锚点
    if (m_anchorUnderMouse) {
        view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    } else {
        view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    }

    view->scale(actualFactor, actualFactor);
    m_currentScale = view->transform().m11();

    // 缩放后约束视口，确保可见区域不超出场景边界
    if (auto *customView = qobject_cast<CustomGraphicsView *>(view)) {
        customView->constrainViewport();
    }

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

void ZoomHandler::setMinScaleFromConstraint(QGraphicsView *view, const QRectF &constraintRect)
{
    if (!view || !view->viewport() || constraintRect.isEmpty()) {
        qDebug() << __FUNCTION__ << __LINE__ << "参数无效，跳过";
        return;
    }

    // 标尺绘制基于完整 viewport 尺寸（mapToScene(viewport->rect())），
    // 因此使用完整 viewport 尺寸计算最小缩放，保证标尺显示范围不超过约束
    qreal minScaleW = static_cast<qreal>(view->viewport()->width())  / constraintRect.width();
    qreal minScaleH = static_cast<qreal>(view->viewport()->height()) / constraintRect.height();
    qreal dynamicMin = qMax(minScaleW, minScaleH);
    qreal oldMinScale = m_minScale;

    // 保留与原始 m_minScale 的较大值
    m_minScale = qMax(m_minScale, dynamicMin);

    qDebug() << __FUNCTION__ << __LINE__
             << "\n  viewport:" << view->viewport()->width() << "x" << view->viewport()->height()
             << "\n  constraintRect:" << constraintRect
             << "\n  minScaleW:" << minScaleW << "minScaleH:" << minScaleH
             << "\n  dynamicMin:" << dynamicMin
             << "\n  m_minScale:" << oldMinScale << "->" << m_minScale;
}
