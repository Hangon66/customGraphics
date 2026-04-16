#include "CustomGraphicsScene.h"
#include "../handlers/CollisionHandler.h"

#include <QPainter>
#include <QtMath>
#include <QLineF>
#include <QMap>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>

CustomGraphicsScene::CustomGraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_gridEnabled(true)
    , m_fineGridSize(20.0)
    , m_coarseGridSize(100.0)
    , m_fineGridColor(230, 230, 230)
    , m_coarseGridColor(200, 200, 200)
    , m_backgroundColor(Qt::white)
    , m_collisionEnabled(false)  // 默认不开启碰撞
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

void CustomGraphicsScene::setCollisionConfig(const CollisionConfig &config)
{
    m_collisionConfig = config;
}

const CollisionConfig& CustomGraphicsScene::collisionConfig() const
{
    return m_collisionConfig;
}

void CustomGraphicsScene::setCollisionEnabled(bool enabled)
{
    m_collisionEnabled = enabled;
}

bool CustomGraphicsScene::isCollisionEnabled() const
{
    return m_collisionEnabled;
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

void CustomGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // 记录所有选中图元的位置
    m_itemOldPositions.clear();

    // 记录当前已选中图元的位置
    QList<QGraphicsItem*> currentSelected = QGraphicsScene::selectedItems();
    for (QGraphicsItem *item : currentSelected) {
        m_itemOldPositions[item] = item->pos();
    }

    // 同时记录点击位置下方的图元位置（处理点击未选中图元的情况）
    QGraphicsItem *itemUnderMouse = itemAt(event->scenePos(), QTransform());
    if (itemUnderMouse && !m_itemOldPositions.contains(itemUnderMouse)) {
        m_itemOldPositions[itemUnderMouse] = itemUnderMouse->pos();
    }

    QGraphicsScene::mousePressEvent(event);
}

void CustomGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // 先调用父类处理，确保选择状态已更新
    QGraphicsScene::mouseReleaseEvent(event);

    // 检查选中图元是否移动，发送信号
    QList<QGraphicsItem*> currentSelected = QGraphicsScene::selectedItems();
    for (QGraphicsItem *item : currentSelected) {
        if (m_itemOldPositions.contains(item)) {
            QPointF oldPos = m_itemOldPositions[item];
            QPointF newPos = item->pos();

            // 如果位置有变化，发送信号
            if (!qFuzzyCompare(oldPos.x(), newPos.x()) ||
                !qFuzzyCompare(oldPos.y(), newPos.y())) {
                emit itemMoved(item, oldPos, newPos);
            }
        }
    }

    m_itemOldPositions.clear();
}

void CustomGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // 如果未启用碰撞阻挡，直接使用默认处理
    if (!m_collisionEnabled) {
        QGraphicsScene::mouseMoveEvent(event);
        return;
    }

    // 获取当前选中的图元
    QList<QGraphicsItem*> selected = QGraphicsScene::selectedItems();
    if (selected.isEmpty()) {
        QGraphicsScene::mouseMoveEvent(event);
        return;
    }

    // 记录移动前的位置
    QMap<QGraphicsItem*, QPointF> posBeforeMove;
    for (QGraphicsItem *item : selected) {
        posBeforeMove[item] = item->pos();
    }

    // 调用父类处理默认移动
    QGraphicsScene::mouseMoveEvent(event);

    // 对每个选中图元进行碰撞检测和阻挡处理
    for (QGraphicsItem *item : selected) {
        if (!CollisionHandler::isCollisionItem(item)) {
            continue;
        }

        // 获取移动前后的位置
        QPointF oldPos = posBeforeMove[item];
        QPointF newPos = item->pos();

        // 如果没有实际移动，跳过
        QPointF delta = newPos - oldPos;
        if (qFuzzyIsNull(delta.x()) && qFuzzyIsNull(delta.y())) {
            continue;
        }

        // 获取源图元类型
        CollisionShapeType sourceType = CollisionHandler::getShapeType(item);

        // 获取场景中其他可能阻挡的图元（考虑碰撞配置）
        QList<QGraphicsItem*> allItems = QGraphicsScene::items();
        QList<QGraphicsItem*> obstacles;
        for (QGraphicsItem *other : allItems) {
            if (!selected.contains(other) &&
                CollisionHandler::isCollisionItemWithConfig(other, sourceType, m_collisionConfig)) {
                obstacles.append(other);
            }
        }

        if (obstacles.isEmpty()) {
            continue;
        }

        // 使用 shape() 的边界矩形进行碰撞检测，确保不包含标签扩展区域
        QRectF itemShapeRect = item->shape().boundingRect();
        QRectF newItemRect = itemShapeRect.translated(newPos);

        for (QGraphicsItem *obstacle : obstacles) {
            QRectF obstacleRect = obstacle->shape().boundingRect().translated(obstacle->pos());

            if (newItemRect.intersects(obstacleRect)) {
                // 发生碰撞，计算沿移动方向的阻挡位置
                QPointF blockedPos = calculateBlockedPosition(
                    itemShapeRect, oldPos, newPos, obstacleRect);

                // 设置阻挡位置
                item->setPos(blockedPos);

                // 更新位置用于后续障碍物检测
                newPos = blockedPos;
                newItemRect = itemShapeRect.translated(newPos);
            }
        }
    }
}

QPointF CustomGraphicsScene::calculateBlockedPosition(
    const QRectF &itemLocalRect, const QPointF &oldPos, const QPointF &newPos, const QRectF &obstacleRect)
{
    // 计算移动方向和距离
    qreal dx = newPos.x() - oldPos.x();
    qreal dy = newPos.y() - oldPos.y();

    // 图元在 oldPos 的场景边界
    QRectF oldSceneRect = itemLocalRect.translated(oldPos);
    // 图元在 newPos 的场景边界
    QRectF newSceneRect = itemLocalRect.translated(newPos);

    // 微小偏移量，避免浮点精度导致的粘连
    const qreal epsilon = 0;

    // 阻挡后的图元位置（初始为 oldPos）
    QPointF blockedPos = oldPos;

    // 检查 oldPos 是否已经与障碍物重叠
    if (oldSceneRect.intersects(obstacleRect)) {
        // 根据移动方向，将图元推出障碍物
        if (qAbs(dx) >= qAbs(dy)) {
            // 主要是水平移动
            if (dx > 0) {
                // 向右移动，从左侧推出
                qreal pushX = obstacleRect.left() - oldSceneRect.right() - epsilon;
                blockedPos.setX(oldPos.x() + pushX);
            } else if (dx < 0) {
                // 向左移动，从右侧推出
                qreal pushX = obstacleRect.right() - oldSceneRect.left() + epsilon;
                blockedPos.setX(oldPos.x() + pushX);
            } else {
                // dx == 0，选择最小推出距离
                qreal pushLeft = obstacleRect.left() - oldSceneRect.right() - epsilon;
                qreal pushRight = obstacleRect.right() - oldSceneRect.left() + epsilon;
                if (qAbs(pushLeft) < qAbs(pushRight)) {
                    blockedPos.setX(oldPos.x() + pushLeft);
                } else {
                    blockedPos.setX(oldPos.x() + pushRight);
                }
            }
            blockedPos.setY(newPos.y());
        } else {
            // 主要是垂直移动
            if (dy > 0) {
                // 向下移动，从上方推出
                qreal pushY = obstacleRect.top() - oldSceneRect.bottom() - epsilon;
                blockedPos.setY(oldPos.y() + pushY);
            } else if (dy < 0) {
                // 向上移动，从下方推出
                qreal pushY = obstacleRect.bottom() - oldSceneRect.top() + epsilon;
                blockedPos.setY(oldPos.y() + pushY);
            } else {
                // dy == 0，选择最小推出距离
                qreal pushUp = obstacleRect.top() - oldSceneRect.bottom() - epsilon;
                qreal pushDown = obstacleRect.bottom() - oldSceneRect.top() + epsilon;
                if (qAbs(pushUp) < qAbs(pushDown)) {
                    blockedPos.setY(oldPos.y() + pushUp);
                } else {
                    blockedPos.setY(oldPos.y() + pushDown);
                }
            }
            blockedPos.setX(newPos.x());
        }
        return blockedPos;
    }

    // 根据移动方向，计算图元可以移动到的最远位置
    if (qAbs(dx) >= qAbs(dy)) {
        // 主要是水平移动
        if (dx > 0) {
            // 向右移动，图元右边缘不能超过障碍物左边缘
            qreal maxRight = obstacleRect.left() - epsilon;
            qreal currentRight = oldSceneRect.right();
            if (currentRight < maxRight) {
                qreal maxDx = maxRight - currentRight;
                blockedPos.setX(oldPos.x() + qMin(dx, maxDx));
            }
            blockedPos.setY(newPos.y());
        } else if (dx < 0) {
            // 向左移动，图元左边缘不能超过障碍物右边缘
            qreal minLeft = obstacleRect.right() + epsilon;
            qreal currentLeft = oldSceneRect.left();
            if (currentLeft > minLeft) {
                qreal maxDx = minLeft - currentLeft;
                blockedPos.setX(oldPos.x() + qMax(dx, maxDx));
            }
            blockedPos.setY(newPos.y());
        }
    } else {
        // 主要是垂直移动
        if (dy > 0) {
            // 向下移动，图元下边缘不能超过障碍物上边缘
            qreal maxBottom = obstacleRect.top() - epsilon;
            qreal currentBottom = oldSceneRect.bottom();
            if (currentBottom < maxBottom) {
                qreal maxDy = maxBottom - currentBottom;
                blockedPos.setY(oldPos.y() + qMin(dy, maxDy));
            }
            blockedPos.setX(newPos.x());
        } else if (dy < 0) {
            // 向上移动，图元上边缘不能超过障碍物下边缘
            qreal minTop = obstacleRect.bottom() + epsilon;
            qreal currentTop = oldSceneRect.top();
            if (currentTop > minTop) {
                qreal maxDy = minTop - currentTop;
                blockedPos.setY(oldPos.y() + qMax(dy, maxDy));
            }
            blockedPos.setX(newPos.x());
        }
    }

    return blockedPos;
}
