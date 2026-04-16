#include "CollisionHandler.h"

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QDebug>

CollisionHandler::CollisionHandler()
{
}

CollisionHandler::~CollisionHandler()
{
}

QGraphicsItem* CollisionHandler::pointInAnyItem(QGraphicsScene *scene, const QPointF &point,
                                                  QGraphicsItem *excludeItem)
{
    if (!scene) {
        return nullptr;
    }

    QList<QGraphicsItem*> itemsAtPoint = scene->items(point);
    for (QGraphicsItem *item : itemsAtPoint) {
        if (item == excludeItem) {
            continue;
        }
        if (isCollisionItem(item)) {
            return item;
        }
    }

    return nullptr;
}

QGraphicsItem* CollisionHandler::rectOverlapsAnyItem(QGraphicsScene *scene, const QRectF &rect,
                                                       QGraphicsItem *excludeItem)
{
    if (!scene) {
        return nullptr;
    }

    QList<QGraphicsItem*> items = scene->items(rect);
    for (QGraphicsItem *item : items) {
        if (item == excludeItem) {
            continue;
        }
        if (isCollisionItem(item)) {
            return item;
        }
    }

    return nullptr;
}

bool CollisionHandler::isCollisionItem(QGraphicsItem *item)
{
    if (!item) {
        return false;
    }

    // 参与碰撞的图元需满足：
    // 1. 可选择（ItemIsSelectable）或可移动（ItemIsMovable）
    // 2. 有 "DrawShape" 或 "CutArea" 数据标记
    QVariant data = item->data(0);
    if (data.isValid()) {
        QString dataType = data.toString();
        if (dataType == "DrawShape" || dataType == "CutArea") {
            return true;
        }
    }

    // 也检查是否设置了 ItemIsMovable 标志
    if (item->flags() & QGraphicsItem::ItemIsMovable) {
        return true;
    }

    return false;
}

QPointF CollisionHandler::calculateBlockedPosition(QGraphicsItem *item, const QPointF &targetPos,
                                                    const QList<QGraphicsItem*> &otherItems)
{
    if (!item || otherItems.isEmpty()) {
        return targetPos;
    }

    QPointF currentPos = item->pos();
    QPointF delta = targetPos - currentPos;

    // 如果没有移动，直接返回
    if (qFuzzyIsNull(delta.x()) && qFuzzyIsNull(delta.y())) {
        return currentPos;
    }

    QRectF itemRect = item->boundingRect();
    QRectF targetRect = itemRect.translated(targetPos);

    QPointF blockedPos = targetPos;

    // 检查每个障碍物
    for (QGraphicsItem *obstacle : otherItems) {
        if (obstacle == item) {
            continue;
        }
        if (!isCollisionItem(obstacle)) {
            continue;
        }

        QRectF obstacleRect = obstacle->boundingRect().translated(obstacle->pos());

        // 如果目标位置与障碍物重叠，计算阻挡位置
        if (targetRect.intersects(obstacleRect)) {
            blockedPos = calculateRectBlockPosition(itemRect.translated(currentPos),
                                                     targetRect, obstacleRect);

            // 更新目标矩形，用于后续障碍物检测
            targetRect = itemRect.translated(blockedPos);

            // 如果仍然重叠，保持当前位置
            if (targetRect.intersects(obstacleRect)) {
                return currentPos;
            }
        }
    }

    return blockedPos;
}

QPointF CollisionHandler::calculateRectBlockPosition(const QRectF &movingRect,
                                                      const QRectF &targetRect,
                                                      const QRectF &obstacleRect)
{
    // 计算移动方向
    qreal dx = targetRect.left() - movingRect.left();
    qreal dy = targetRect.top() - movingRect.top();

    QPointF blockedPos(targetRect.left(), targetRect.top());

    // 根据移动方向，选择阻挡方式
    if (qAbs(dx) >= qAbs(dy)) {
        // 主要是水平移动
        if (dx > 0) {
            // 向右移动，被障碍物左边缘阻挡
            qreal blockedLeft = obstacleRect.left() - movingRect.width();
            blockedPos.setX(qMin(targetRect.left(), blockedLeft));
        } else {
            // 向左移动，被障碍物右边缘阻挡
            qreal blockedLeft = obstacleRect.right();
            blockedPos.setX(qMax(targetRect.left(), blockedLeft));
        }
    } else {
        // 主要是垂直移动
        if (dy > 0) {
            // 向下移动，被障碍物上边缘阻挡
            qreal blockedTop = obstacleRect.top() - movingRect.height();
            blockedPos.setY(qMin(targetRect.top(), blockedTop));
        } else {
            // 向上移动，被障碍物下边缘阻挡
            qreal blockedTop = obstacleRect.bottom();
            blockedPos.setY(qMax(targetRect.top(), blockedTop));
        }
    }

    return blockedPos;
}

QPointF CollisionHandler::calculateSingleBlock(const QRectF &movingRect,
                                                const QRectF &obstacleRect,
                                                const QPointF &delta)
{
    QPointF blockOffset(0, 0);

    // 计算在各个方向上需要的最小偏移量来消除重叠
    qreal overlapLeft = obstacleRect.right() - movingRect.left();   // 向左移出
    qreal overlapRight = movingRect.right() - obstacleRect.left();  // 向右移出
    qreal overlapTop = obstacleRect.bottom() - movingRect.top();    // 向上移出
    qreal overlapBottom = movingRect.bottom() - obstacleRect.top(); // 向下移出

    // 选择最小偏移方向（根据移动方向）
    if (qAbs(delta.x()) > qAbs(delta.y())) {
        // 主要是水平移动
        if (delta.x() > 0) {
            blockOffset.setX(-overlapRight);
        } else {
            blockOffset.setX(overlapLeft);
        }
    } else {
        // 主要是垂直移动
        if (delta.y() > 0) {
            blockOffset.setY(-overlapBottom);
        } else {
            blockOffset.setY(overlapTop);
        }
    }

    return blockOffset;
}
