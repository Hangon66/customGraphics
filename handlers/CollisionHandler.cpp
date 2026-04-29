#include "CollisionHandler.h"
#include "../view/ShapeMetadata.h"

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

void CollisionConfig::setCollisionEnabled(ShapeMeta::Type type1, ShapeMeta::Type type2, bool enabled)
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

bool CollisionConfig::isCollisionEnabled(ShapeMeta::Type type1, ShapeMeta::Type type2) const
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
    int maxType = static_cast<int>(ShapeMeta::Unknown);
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
    setCollisionEnabled(ShapeMeta::Rect, ShapeMeta::Rect, true);
}

void CollisionConfig::enableLineOnlyCollision()
{
    disableAllCollisions();
    setCollisionEnabled(ShapeMeta::Line, ShapeMeta::Line, true);
}

void CollisionConfig::enableRectAndLineCollision()
{
    disableAllCollisions();
    setCollisionEnabled(ShapeMeta::Rect, ShapeMeta::Rect, true);
    setCollisionEnabled(ShapeMeta::Line, ShapeMeta::Line, true);
    setCollisionEnabled(ShapeMeta::Rect, ShapeMeta::Line, true);
}

void CollisionConfig::setCollisionMargin(qreal margin)
{
    m_collisionMargin = qMax(0.0, margin);
}

qreal CollisionConfig::collisionMargin() const
{
    return m_collisionMargin;
}

void CollisionConfig::setCollisionMarginMM(qreal mm)
{
    m_collisionMargin = qMax(0.0, mm * m_mmToSceneScale);
}

qreal CollisionConfig::collisionMarginMM() const
{
    if (qFuzzyIsNull(m_mmToSceneScale)) {
        return 0.0;
    }
    return m_collisionMargin / m_mmToSceneScale;
}

void CollisionConfig::setMmToSceneScale(qreal scale)
{
    m_mmToSceneScale = (scale > 0.0) ? scale : 1.0;
}

qreal CollisionConfig::mmToSceneScale() const
{
    return m_mmToSceneScale;
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

    // 获取碰撞边距
    qreal margin = (config ? config->collisionMargin() : 0.0);

    QList<QGraphicsItem*> itemsAtPoint = scene->items(point);
    for (QGraphicsItem *item : itemsAtPoint) {
        if (item == excludeItem) {
            continue;
        }

        // 先检查是否参与碰撞
        if (!isCollisionItem(item)) {
            continue;
        }

        // 无边距时直接返回匹配项
        if (qFuzzyIsNull(margin)) {
            return item;
        }

        // 有边距时，检查点是否在障碍物扩展区域内
        QRectF itemRect = item->sceneBoundingRect();
        QRectF expandedRect = itemRect.adjusted(-margin, -margin, margin, margin);
        if (expandedRect.contains(point)) {
            return item;
        }
    }

    // 有边距时，还需检查附近图元（点可能不在图元上但在扩展区域内）
    if (!qFuzzyIsNull(margin)) {
        QRectF searchRect(point.x() - margin, point.y() - margin, margin * 2, margin * 2);
        QList<QGraphicsItem*> nearbyItems = scene->items(searchRect);
        for (QGraphicsItem *item : nearbyItems) {
            if (item == excludeItem) {
                continue;
            }
            if (!isCollisionItem(item)) {
                continue;
            }
            // 已在上方检查过的跳过
            if (itemsAtPoint.contains(item)) {
                continue;
            }

            QRectF itemRect = item->sceneBoundingRect();
            QRectF expandedRect = itemRect.adjusted(-margin, -margin, margin, margin);
            if (expandedRect.contains(point)) {
                return item;
            }
        }
    }

    return nullptr;
}

QGraphicsItem* CollisionHandler::pointInAnyItemWithConfig(QGraphicsScene *scene, const QPointF &point,
                                                            QGraphicsItem *excludeItem,
                                                            ShapeMeta::Type sourceType,
                                                            const CollisionConfig &config)
{
    if (!scene) {
        return nullptr;
    }

    qreal margin = config.collisionMargin();

    QList<QGraphicsItem*> itemsAtPoint = scene->items(point);
    for (QGraphicsItem *item : itemsAtPoint) {
        if (item == excludeItem) {
            continue;
        }

        // 先检查是否参与碰撞
        if (!isCollisionItem(item)) {
            continue;
        }

        // 检查碰撞配置
        if (!isCollisionItemWithConfig(item, sourceType, config)) {
            continue;
        }

        // 无边距时直接返回匹配项
        if (qFuzzyIsNull(margin)) {
            return item;
        }

        // 有边距时，检查点是否在障碍物扩展区域内
        QRectF itemRect = item->sceneBoundingRect();
        QRectF expandedRect = itemRect.adjusted(-margin, -margin, margin, margin);
        if (expandedRect.contains(point)) {
            return item;
        }
    }

    // 有边距时，还需检查附近图元
    if (!qFuzzyIsNull(margin)) {
        QRectF searchRect(point.x() - margin, point.y() - margin, margin * 2, margin * 2);
        QList<QGraphicsItem*> nearbyItems = scene->items(searchRect);
        for (QGraphicsItem *item : nearbyItems) {
            if (item == excludeItem) {
                continue;
            }
            if (!isCollisionItem(item)) {
                continue;
            }
            if (!isCollisionItemWithConfig(item, sourceType, config)) {
                continue;
            }
            // 已在上方检查过的跳过
            if (itemsAtPoint.contains(item)) {
                continue;
            }

            QRectF itemRect = item->sceneBoundingRect();
            QRectF expandedRect = itemRect.adjusted(-margin, -margin, margin, margin);
            if (expandedRect.contains(point)) {
                return item;
            }
        }
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

    // 获取碰撞边距
    qreal margin = (config ? config->collisionMargin() : 0.0);

    // 有边距时，扩大搜索区域以包含附近图元
    QRectF searchRect = rect;
    if (!qFuzzyIsNull(margin)) {
        searchRect = rect.adjusted(-margin, -margin, margin, margin);
    }

    QList<QGraphicsItem*> items = scene->items(searchRect);
    for (QGraphicsItem *item : items) {
        if (item == excludeItem) {
            continue;
        }

        // 先检查是否参与碰撞
        if (!isCollisionItem(item)) {
            continue;
        }

        // 如果有配置，使用配置检查
        if (config) {
            ShapeMeta::Type itemType = getShapeType(item);
            // 对于矩形重叠检测，假设源类型为 Rect
            if (!config->isCollisionEnabled(itemType, ShapeMeta::Rect)) {
                continue;
            }
        }

        // 使用 intersects 区分边界接触和真正重叠
        // QRectF::intersects 在边界接触时返回 false，只有真正重叠才返回 true
        // 如果有边距，障碍物边界向四周扩展
        QRectF itemRect = item->sceneBoundingRect();
        if (!qFuzzyIsNull(margin)) {
            itemRect = itemRect.adjusted(-margin, -margin, margin, margin);
        }
        if (rect.intersects(itemRect)) {
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
    QVariant data = item->data(ShapeMeta::Category);
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
                                                   ShapeMeta::Type sourceType,
                                                   const CollisionConfig &config)
{
    if (!isCollisionItem(item)) {
        return false;
    }

    ShapeMeta::Type itemType = getShapeType(item);
    return config.isCollisionEnabled(sourceType, itemType);
}

ShapeMeta::Type CollisionHandler::getShapeType(QGraphicsItem *item)
{
    if (!item) {
        return ShapeMeta::Unknown;
    }

    // 从 data(ShapeMeta::ShapeType) 获取类型
    QVariant typeData = item->data(ShapeMeta::ShapeType);
    if (typeData.isValid()) {
        int type = typeData.toInt();
        switch (type) {
        case ShapeMeta::Rect:
        case ShapeMeta::Line:
        case ShapeMeta::Pixmap:
            return static_cast<ShapeMeta::Type>(type);
        default:
            return ShapeMeta::Unknown;
        }
    }

    // 根据图元类型推断
    if (qgraphicsitem_cast<QGraphicsRectItem*>(item)) {
        return ShapeMeta::Rect;
    }
    if (qgraphicsitem_cast<QGraphicsLineItem*>(item)) {
        return ShapeMeta::Line;
    }

    return ShapeMeta::Unknown;
}

QPointF CollisionHandler::calculateBlockedPosition(QGraphicsItem *item, const QPointF &targetPos,
                                                    const QList<QGraphicsItem*> &otherItems,
                                                    qreal margin)
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

    QRectF currentSceneRect = item->sceneBoundingRect();
    QRectF targetRect = currentSceneRect.translated(delta);

    QPointF blockedPos = targetPos;

    // 检查每个障碍物
    for (QGraphicsItem *obstacle : otherItems) {
        if (obstacle == item) {
            continue;
        }
        if (!isCollisionItem(obstacle)) {
            continue;
        }

        QRectF obstacleRect = obstacle->sceneBoundingRect();
        // 有边距时扩展障碍物区域
        if (!qFuzzyIsNull(margin)) {
            obstacleRect = obstacleRect.adjusted(-margin, -margin, margin, margin);
        }

        // 如果目标位置与障碍物重叠，计算阻挡位置
        if (targetRect.intersects(obstacleRect)) {
            blockedPos = calculateRectBlockPosition(currentSceneRect,
                                                     targetRect, obstacleRect, margin);

            // 更新目标矩形，用于后续障碍物检测
            targetRect = currentSceneRect.translated(blockedPos - currentPos);

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
                                                      const QRectF &obstacleRect,
                                                      qreal margin)
{
    Q_UNUSED(margin)

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
