#include "CollisionHandler.h"

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QDebug>

// ==================== CollisionConfig 实现 ====================

CollisionConfig::CollisionConfig()
{
    // 默认启用所有碰撞
    enableAllCollisions();
}

CollisionConfig::~CollisionConfig()
{
}

void CollisionConfig::setCollisionEnabled(CollisionShapeType type1, CollisionShapeType type2, bool enabled)
{
    // 确保类型对的顺序一致，(type1, type2) 和 (type2, type1) 视为相同
    int t1 = static_cast<int>(type1);
    int t2 = static_cast<int>(type2);
    QPair<int, int> pair(qMin(t1, t2), qMax(t1, t2));

    if (enabled) {
        m_enabledPairs.insert(pair);
    } else {
        m_enabledPairs.remove(pair);
    }
}

bool CollisionConfig::isCollisionEnabled(CollisionShapeType type1, CollisionShapeType type2) const
{
    int t1 = static_cast<int>(type1);
    int t2 = static_cast<int>(type2);
    QPair<int, int> pair(qMin(t1, t2), qMax(t1, t2));

    return m_enabledPairs.contains(pair);
}

void CollisionConfig::enableAllCollisions()
{
    m_enabledPairs.clear();
    // 添加所有类型对
    int maxType = static_cast<int>(CollisionShapeType::Unknown);
    for (int i = 0; i <= maxType; ++i) {
        for (int j = i; j <= maxType; ++j) {
            m_enabledPairs.insert(qMakePair(i, j));
        }
    }
}

void CollisionConfig::disableAllCollisions()
{
    m_enabledPairs.clear();
}

void CollisionConfig::enableRectOnlyCollision()
{
    disableAllCollisions();
    setCollisionEnabled(CollisionShapeType::Rect, CollisionShapeType::Rect, true);
}

void CollisionConfig::enableLineOnlyCollision()
{
    disableAllCollisions();
    setCollisionEnabled(CollisionShapeType::Line, CollisionShapeType::Line, true);
}

void CollisionConfig::enableRectAndLineCollision()
{
    disableAllCollisions();
    setCollisionEnabled(CollisionShapeType::Rect, CollisionShapeType::Rect, true);
    setCollisionEnabled(CollisionShapeType::Line, CollisionShapeType::Line, true);
    setCollisionEnabled(CollisionShapeType::Rect, CollisionShapeType::Line, true);
}

// ==================== CollisionHandler 实现 ====================

CollisionHandler::CollisionHandler()
{
}

CollisionHandler::~CollisionHandler()
{
}

QGraphicsItem* CollisionHandler::pointInAnyItem(QGraphicsScene *scene, const QPointF &point,
                                                  QGraphicsItem *excludeItem,
                                                  const CollisionConfig *config)
{
    if (!scene) {
        return nullptr;
    }

    QList<QGraphicsItem*> itemsAtPoint = scene->items(point);
    for (QGraphicsItem *item : itemsAtPoint) {
        if (item == excludeItem) {
            continue;
        }

        // 如果有配置，使用配置检查
        if (config) {
            CollisionShapeType itemType = getShapeType(item);
            // 对于点检测，假设源类型为 Unknown（允许所有）
            if (!config->isCollisionEnabled(itemType, CollisionShapeType::Unknown)) {
                if (!isCollisionItem(item)) {
                    continue;
                }
            }
        } else {
            if (!isCollisionItem(item)) {
                continue;
            }
        }

        return item;
    }

    return nullptr;
}

QGraphicsItem* CollisionHandler::rectOverlapsAnyItem(QGraphicsScene *scene, const QRectF &rect,
                                                       QGraphicsItem *excludeItem,
                                                       const CollisionConfig *config)
{
    if (!scene) {
        return nullptr;
    }

    QList<QGraphicsItem*> items = scene->items(rect);
    for (QGraphicsItem *item : items) {
        if (item == excludeItem) {
            continue;
        }

        // 如果有配置，使用配置检查
        if (config) {
            CollisionShapeType itemType = getShapeType(item);
            // 对于矩形重叠检测，假设源类型为 Rect
            if (!config->isCollisionEnabled(itemType, CollisionShapeType::Rect)) {
                if (!isCollisionItem(item)) {
                    continue;
                }
            }
        } else {
            if (!isCollisionItem(item)) {
                continue;
            }
        }

        return item;
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

bool CollisionHandler::isCollisionItemWithConfig(QGraphicsItem *item,
                                                   CollisionShapeType sourceType,
                                                   const CollisionConfig &config)
{
    if (!isCollisionItem(item)) {
        return false;
    }

    CollisionShapeType itemType = getShapeType(item);
    return config.isCollisionEnabled(sourceType, itemType);
}

CollisionShapeType CollisionHandler::getShapeType(QGraphicsItem *item)
{
    if (!item) {
        return CollisionShapeType::Unknown;
    }

    // 从 data(1) 获取类型
    QVariant typeData = item->data(1);
    if (typeData.isValid()) {
        int type = typeData.toInt();
        switch (type) {
        case 0: // ShapeType::Rect
            return CollisionShapeType::Rect;
        case 1: // ShapeType::Line
            return CollisionShapeType::Line;
        default:
            return CollisionShapeType::Unknown;
        }
    }

    // 根据图元类型推断
    if (qgraphicsitem_cast<QGraphicsRectItem*>(item)) {
        return CollisionShapeType::Rect;
    }
    if (qgraphicsitem_cast<QGraphicsLineItem*>(item)) {
        return CollisionShapeType::Line;
    }

    return CollisionShapeType::Unknown;
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
