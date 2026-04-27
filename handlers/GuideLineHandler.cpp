#include "GuideLineHandler.h"

#include <QGraphicsView>
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>

GuideLineHandler::GuideLineHandler(int priority, QObject *parent)
    : AbstractInteractionHandler(priority, parent)
    , m_guideLineVisible(true)
    , m_dragState(DragState::None)
    , m_dragGuideLine({GuideLine::Horizontal, 0.0})
    , m_dragIndex(-1)
    , m_rulerWidth(25)
    , m_guideLineColor(0, 188, 212)      // 青色 #00BCD4
    , m_dragColor(233, 30, 99)            // 品红色 #E91E63
    , m_snapThreshold(5.0)
    , m_rulerPosition(RulerPosition::Bottom)
{
}

GuideLineHandler::~GuideLineHandler()
{
}

QString GuideLineHandler::handlerName() const
{
    return QStringLiteral("GuideLineHandler");
}

// ========== 事件处理 ==========

bool GuideLineHandler::handleMousePress(QGraphicsView *view, QMouseEvent *event)
{
    if (!view || m_dragState != DragState::None) {
        return false;
    }

    // 仅处理左键
    if (event->button() != Qt::LeftButton) {
        return false;
    }

    QPoint viewPos = view->mapFromGlobal(event->globalPos());

    // 1. 检测是否在标尺区域（从标尺拖出创建辅助线）
    if (isInHorizontalRuler(viewPos, view)) {
        // 从水平标尺拖出水平辅助线
        QPointF scenePos = view->mapToScene(viewPos);
        m_dragGuideLine.type = GuideLine::Horizontal;
        m_dragGuideLine.position = scenePos.y();
        m_dragState = DragState::Creating;
        view->viewport()->setCursor(Qt::SplitVCursor);
        view->viewport()->update();
        return true;
    }

    if (isInLeftRuler(viewPos, view)) {
        // 从左侧标尺拖出垂直辅助线
        QPointF scenePos = view->mapToScene(viewPos);
        m_dragGuideLine.type = GuideLine::Vertical;
        m_dragGuideLine.position = scenePos.x();
        m_dragState = DragState::Creating;
        view->viewport()->setCursor(Qt::SplitHCursor);
        view->viewport()->update();
        return true;
    }

    // 2. 检测是否点击了已有的辅助线（移动辅助线）
    int idx = findGuideLineAt(viewPos, view);
    if (idx >= 0) {
        m_dragIndex = idx;
        m_dragGuideLine = m_guideLines[idx];
        m_dragState = DragState::Moving;
        if (m_guideLines[idx].type == GuideLine::Horizontal) {
            view->viewport()->setCursor(Qt::SplitVCursor);
        } else {
            view->viewport()->setCursor(Qt::SplitHCursor);
        }
        view->viewport()->update();
        return true;
    }

    return false;
}

bool GuideLineHandler::handleMouseMove(QGraphicsView *view, QMouseEvent *event)
{
    if (!view || m_dragState == DragState::None) {
        return false;
    }

    QPoint viewPos = view->mapFromGlobal(event->globalPos());
    QPointF scenePos = view->mapToScene(viewPos);

    // 更新拖拽中的辅助线位置
    if (m_dragGuideLine.type == GuideLine::Horizontal) {
        m_dragGuideLine.position = scenePos.y();
    } else {
        m_dragGuideLine.position = scenePos.x();
    }

    view->viewport()->update();
    return true;
}

bool GuideLineHandler::handleMouseRelease(QGraphicsView *view, QMouseEvent *event)
{
    if (!view || m_dragState == DragState::None) {
        return false;
    }

    QPoint viewPos = view->mapFromGlobal(event->globalPos());

    // 检查是否释放在标尺区域内（删除辅助线）
    bool inRuler = isInHorizontalRuler(viewPos, view) || isInLeftRuler(viewPos, view);

    if (m_dragState == DragState::Creating) {
        if (!inRuler) {
            // 在场景区域内释放，创建辅助线
            addGuideLine(m_dragGuideLine.type, m_dragGuideLine.position);
        }
        // 在标尺区域释放则取消创建
    } else if (m_dragState == DragState::Moving) {
        if (inRuler) {
            // 拖拽到标尺区域，删除辅助线
            if (m_dragIndex >= 0 && m_dragIndex < m_guideLines.size()) {
                removeGuideLine(m_dragIndex);
            }
        } else {
            // 更新辅助线位置
            if (m_dragIndex >= 0 && m_dragIndex < m_guideLines.size()) {
                m_guideLines[m_dragIndex].position = m_dragGuideLine.position;
                emit guideLinesChanged();
            }
        }
    }

    // 重置拖拽状态
    m_dragState = DragState::None;
    m_dragIndex = -1;
    view->viewport()->setCursor(Qt::ArrowCursor);
    view->viewport()->update();

    return true;
}

// ========== 绘制 ==========

void GuideLineHandler::paint(QPainter *painter, QGraphicsView *view)
{
    if (!m_guideLineVisible || !view || !painter) {
        return;
    }

    painter->save();

    QRect viewportRect = view->viewport()->rect();
    QPointF sceneTopLeft = view->mapToScene(viewportRect.topLeft());
    QPointF sceneBottomRight = view->mapToScene(viewportRect.bottomRight());

    // 计算辅助线绘制的 Y 范围（根据标尺位置调整）
    int topLimit = 0;
    int bottomLimit = viewportRect.bottom();
    if (m_rulerPosition == RulerPosition::Top) {
        topLimit = m_rulerWidth;
    } else {
        bottomLimit = viewportRect.bottom() - m_rulerWidth;
    }

    // 绘制已有辅助线
    QPen normalPen(m_guideLineColor, 1, Qt::DashLine);
    normalPen.setCosmetic(true);
    painter->setPen(normalPen);

    for (int i = 0; i < m_guideLines.size(); ++i) {
        const GuideLine &gl = m_guideLines[i];

        // 拖拽移动中的辅助线用高亮色绘制，跳过此处
        if (m_dragState == DragState::Moving && i == m_dragIndex) {
            continue;
        }

        if (gl.type == GuideLine::Horizontal) {
            // 仅绘制在可见范围内的辅助线
            if (gl.position >= sceneTopLeft.y() && gl.position <= sceneBottomRight.y()) {
                int viewY = view->mapFromScene(QPointF(0, gl.position)).y();
                if (viewY >= topLimit && viewY <= bottomLimit) {
                    painter->drawLine(m_rulerWidth, viewY, viewportRect.right(), viewY);
                }
            }
        } else {
            if (gl.position >= sceneTopLeft.x() && gl.position <= sceneBottomRight.x()) {
                int viewX = view->mapFromScene(QPointF(gl.position, 0)).x();
                painter->drawLine(viewX, topLimit, viewX, bottomLimit);
            }
        }
    }

    // 绘制拖拽中的辅助线（高亮色）
    QPen dragPen(m_dragColor, 1, Qt::DashLine);
    dragPen.setCosmetic(true);
    painter->setPen(dragPen);

    if (m_dragState != DragState::None) {
        if (m_dragGuideLine.type == GuideLine::Horizontal) {
            int viewY = view->mapFromScene(QPointF(0, m_dragGuideLine.position)).y();
            if (viewY >= topLimit && viewY <= bottomLimit) {
                painter->drawLine(m_rulerWidth, viewY, viewportRect.right(), viewY);
            }
        } else {
            int viewX = view->mapFromScene(QPointF(m_dragGuideLine.position, 0)).x();
            painter->drawLine(viewX, topLimit, viewX, bottomLimit);
        }
    }

    // 绘制标尺上的三角形标记
    paintRulerIndicators(painter, view);

    painter->restore();
}

void GuideLineHandler::paintRulerIndicators(QPainter *painter, QGraphicsView *view)
{
    // 绘制标尺区域内的辅助线小三角形标记
    // 水平辅助线 → 在左侧标尺上绘制向右三角形（指示 Y 位置）
    // 垂直辅助线 → 在水平标尺上绘制三角形（指示 X 位置）
    QPen indicatorPen(m_guideLineColor, 1);
    indicatorPen.setCosmetic(true);
    painter->setPen(indicatorPen);
    painter->setBrush(m_guideLineColor);

    const int triSize = 6; // 三角形大小
    int viewportH = view->viewport()->height();
    int viewportW = view->viewport()->width();

    for (const GuideLine &gl : m_guideLines) {
        if (gl.type == GuideLine::Horizontal) {
            int viewY = view->mapFromScene(QPointF(0, gl.position)).y();

            if (m_rulerPosition == RulerPosition::Bottom) {
                // 底部标尺：左侧标尺 Y 范围 0 ~ viewportH - m_rulerWidth
                if (viewY >= 0 && viewY <= viewportH - m_rulerWidth) {
                    QPolygon triangle;
                    triangle << QPoint(0, viewY - triSize / 2)
                             << QPoint(triSize, viewY)
                             << QPoint(0, viewY + triSize / 2);
                    painter->drawPolygon(triangle);
                }
            } else {
                // 顶部标尺：左侧标尺 Y 范围 m_rulerWidth ~ viewportH
                if (viewY >= m_rulerWidth && viewY <= viewportH) {
                    QPolygon triangle;
                    triangle << QPoint(0, viewY - triSize / 2)
                             << QPoint(triSize, viewY)
                             << QPoint(0, viewY + triSize / 2);
                    painter->drawPolygon(triangle);
                }
            }
        } else {
            int viewX = view->mapFromScene(QPointF(gl.position, 0)).x();

            if (viewX < m_rulerWidth || viewX > viewportW) {
                continue;
            }

            if (m_rulerPosition == RulerPosition::Bottom) {
                // 底部标尺上绘制向上三角形
                int bottomY = viewportH - m_rulerWidth;
                QPolygon triangle;
                triangle << QPoint(viewX - triSize / 2, viewportH)
                         << QPoint(viewX, bottomY)
                         << QPoint(viewX + triSize / 2, viewportH);
                painter->drawPolygon(triangle);
            } else {
                // 顶部标尺上绘制向下三角形
                QPolygon triangle;
                triangle << QPoint(viewX - triSize / 2, 0)
                         << QPoint(viewX, triSize)
                         << QPoint(viewX + triSize / 2, 0);
                painter->drawPolygon(triangle);
            }
        }
    }

    painter->setBrush(Qt::NoBrush);
}

// ========== 辅助线管理 ==========

void GuideLineHandler::addGuideLine(GuideLine::Type type, qreal position)
{
    m_guideLines.append({type, position});
    emit guideLinesChanged();
}

void GuideLineHandler::removeGuideLine(int index)
{
    if (index >= 0 && index < m_guideLines.size()) {
        m_guideLines.removeAt(index);
        emit guideLinesChanged();
    }
}

void GuideLineHandler::clearGuideLines()
{
    if (!m_guideLines.isEmpty()) {
        m_guideLines.clear();
        emit guideLinesChanged();
    }
}

int GuideLineHandler::guideLineCount() const
{
    return m_guideLines.size();
}

GuideLine GuideLineHandler::guideLine(int index) const
{
    if (index >= 0 && index < m_guideLines.size()) {
        return m_guideLines[index];
    }
    return {GuideLine::Horizontal, 0.0};
}

void GuideLineHandler::setGuideLineVisible(bool visible)
{
    if (m_guideLineVisible != visible) {
        m_guideLineVisible = visible;
    }
}

bool GuideLineHandler::isGuideLineVisible() const
{
    return m_guideLineVisible;
}

void GuideLineHandler::setRulerWidth(int width)
{
    m_rulerWidth = qMax(0, width);
}

int GuideLineHandler::rulerWidth() const
{
    return m_rulerWidth;
}

void GuideLineHandler::setGuideLineColor(const QColor &color)
{
    m_guideLineColor = color;
}

QColor GuideLineHandler::guideLineColor() const
{
    return m_guideLineColor;
}

void GuideLineHandler::setDragColor(const QColor &color)
{
    m_dragColor = color;
}

QColor GuideLineHandler::dragColor() const
{
    return m_dragColor;
}

void GuideLineHandler::setSnapThreshold(qreal threshold)
{
    m_snapThreshold = qMax(1.0, threshold);
}

qreal GuideLineHandler::snapThreshold() const
{
    return m_snapThreshold;
}

void GuideLineHandler::setRulerPosition(RulerPosition position)
{
    m_rulerPosition = position;
}

GuideLineHandler::RulerPosition GuideLineHandler::rulerPosition() const
{
    return m_rulerPosition;
}

// ========== 内部工具方法 ==========

bool GuideLineHandler::isInHorizontalRuler(const QPoint &viewPos, QGraphicsView *view) const
{
    Q_UNUSED(view)
    int viewportH = view->viewport()->height();
    if (m_rulerPosition == RulerPosition::Bottom) {
        return viewPos.y() >= viewportH - m_rulerWidth && viewPos.y() < viewportH
               && viewPos.x() >= m_rulerWidth;
    } else {
        return viewPos.y() >= 0 && viewPos.y() < m_rulerWidth && viewPos.x() >= m_rulerWidth;
    }
}

bool GuideLineHandler::isInLeftRuler(const QPoint &viewPos, QGraphicsView *view) const
{
    Q_UNUSED(view)
    int viewportH = view->viewport()->height();
    if (m_rulerPosition == RulerPosition::Bottom) {
        // 左侧标尺，排除左下角区域（由水平标尺覆盖）
        return viewPos.x() >= 0 && viewPos.x() < m_rulerWidth
               && viewPos.y() >= 0 && viewPos.y() < viewportH - m_rulerWidth;
    } else {
        // 顶部标尺时，排除左上角
        return viewPos.x() >= 0 && viewPos.x() < m_rulerWidth
               && viewPos.y() >= m_rulerWidth;
    }
}

int GuideLineHandler::findGuideLineAt(const QPoint &viewPos, QGraphicsView *view) const
{
    if (!view) {
        return -1;
    }

    QPointF scenePos = view->mapToScene(viewPos);
    int viewportH = view->viewport()->height();

    // 计算辅助线可点击的 Y 范围
    int yMin = 0;
    int yMax = viewportH;
    if (m_rulerPosition == RulerPosition::Top) {
        yMin = m_rulerWidth;
    } else {
        yMax = viewportH - m_rulerWidth;
    }

    for (int i = 0; i < m_guideLines.size(); ++i) {
        const GuideLine &gl = m_guideLines[i];

        if (gl.type == GuideLine::Horizontal) {
            // 计算辅助线在视图中的 Y 坐标
            int lineViewY = view->mapFromScene(QPointF(0, gl.position)).y();
            if (qAbs(viewPos.y() - lineViewY) <= m_snapThreshold &&
                viewPos.y() >= yMin && viewPos.y() <= yMax &&
                viewPos.x() >= m_rulerWidth) {
                return i;
            }
        } else {
            // 计算辅助线在视图中的 X 坐标
            int lineViewX = view->mapFromScene(QPointF(gl.position, 0)).x();
            if (qAbs(viewPos.x() - lineViewX) <= m_snapThreshold &&
                viewPos.x() >= m_rulerWidth &&
                viewPos.y() >= yMin && viewPos.y() <= yMax) {
                return i;
            }
        }
    }

    return -1;
}
