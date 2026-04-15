#include "CustomGraphicsScene.h"

#include <QPainter>
#include <QtMath>

CustomGraphicsScene::CustomGraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_gridEnabled(true)
    , m_fineGridSize(20.0)
    , m_coarseGridSize(100.0)
    , m_fineGridColor(230, 230, 230)
    , m_coarseGridColor(200, 200, 200)
    , m_backgroundColor(Qt::white)
{
    setSceneRect(-5000, -5000, 10000, 10000);
}

void CustomGraphicsScene::setGridEnabled(bool enabled)
{
    if (m_gridEnabled != enabled) {
        m_gridEnabled = enabled;
        invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
    }
}

bool CustomGraphicsScene::isGridEnabled() const
{
    return m_gridEnabled;
}

void CustomGraphicsScene::setFineGridSize(qreal size)
{
    if (size > 0 && !qFuzzyCompare(m_fineGridSize, size)) {
        m_fineGridSize = size;
        invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
    }
}

qreal CustomGraphicsScene::fineGridSize() const
{
    return m_fineGridSize;
}

void CustomGraphicsScene::setCoarseGridSize(qreal size)
{
    if (size > 0 && !qFuzzyCompare(m_coarseGridSize, size)) {
        m_coarseGridSize = size;
        invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
    }
}

qreal CustomGraphicsScene::coarseGridSize() const
{
    return m_coarseGridSize;
}

void CustomGraphicsScene::setGridColors(const QColor &fine, const QColor &coarse)
{
    m_fineGridColor = fine;
    m_coarseGridColor = coarse;
    invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
}

void CustomGraphicsScene::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
}

void CustomGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->fillRect(rect, m_backgroundColor);

    if (!m_gridEnabled) {
        return;
    }

    // 绘制细网格
    {
        QPen pen(m_fineGridColor);
        pen.setCosmetic(true);
        pen.setWidthF(0.5);
        painter->setPen(pen);

        const qreal left = qFloor(rect.left() / m_fineGridSize) * m_fineGridSize;
        const qreal top = qFloor(rect.top() / m_fineGridSize) * m_fineGridSize;

        QVector<QLineF> lines;
        for (qreal x = left; x <= rect.right(); x += m_fineGridSize) {
            lines.append(QLineF(x, rect.top(), x, rect.bottom()));
        }
        for (qreal y = top; y <= rect.bottom(); y += m_fineGridSize) {
            lines.append(QLineF(rect.left(), y, rect.right(), y));
        }
        painter->drawLines(lines);
    }

    // 绘制粗网格
    {
        QPen pen(m_coarseGridColor);
        pen.setCosmetic(true);
        pen.setWidthF(1.0);
        painter->setPen(pen);

        const qreal left = qFloor(rect.left() / m_coarseGridSize) * m_coarseGridSize;
        const qreal top = qFloor(rect.top() / m_coarseGridSize) * m_coarseGridSize;

        QVector<QLineF> lines;
        for (qreal x = left; x <= rect.right(); x += m_coarseGridSize) {
            lines.append(QLineF(x, rect.top(), x, rect.bottom()));
        }
        for (qreal y = top; y <= rect.bottom(); y += m_coarseGridSize) {
            lines.append(QLineF(rect.left(), y, rect.right(), y));
        }
        painter->drawLines(lines);
    }
}
