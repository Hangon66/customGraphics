#include "CustomGraphicsScene.h"
#include "../handlers/CollisionHandler.h"

#include <QPainter>
#include <QtMath>
#include <QLineF>
#include <QMap>
#include <QDebug>
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

void CustomGraphicsScene::setBoundaryConstraint(const QRectF &boundary)
{
    m_boundaryConstraint = boundary;
    emit boundaryConstraintChanged(boundary);
}

void CustomGraphicsScene::clearBoundaryConstraint()
{
    m_boundaryConstraint = QRectF();
    emit boundaryConstraintChanged(m_boundaryConstraint);
}

QRectF CustomGraphicsScene::boundaryConstraint() const
{
    return m_boundaryConstraint;
}

bool CustomGraphicsScene::hasBoundaryConstraint() const
{
    return !m_boundaryConstraint.isNull();
}

// ========== 背景图片 ==========

void CustomGraphicsScene::setBackgroundPixmap(const QPixmap &pixmap)
{
    m_backgroundPixmap = pixmap;

    if (!pixmap.isNull()) {
        // 自动设置边界约束为图片大小
        QRectF pixmapRect(0, 0, pixmap.width(), pixmap.height());
        m_boundaryConstraint = pixmapRect;
        emit boundaryConstraintChanged(pixmapRect);
        // 场景大小保持足够大，允许视图拖拽到空白区域
        // 设置一个足够大的场景区域，以背景图片为中心
        qreal margin = 5000;  // 边距，允许拖拽的范围
        setSceneRect(-margin, -margin,
                     pixmap.width() + margin * 2,
                     pixmap.height() + margin * 2);
    } else {
        // 清除背景图片时同时清除边界约束
        m_boundaryConstraint = QRectF();
        setSceneRect(-5000, -5000, 10000, 10000);
        emit boundaryConstraintChanged(m_boundaryConstraint);
    }

    invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
}

bool CustomGraphicsScene::loadBackgroundFromFile(const QString &filePath)
{
    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        qWarning() << "Failed to load background pixmap:" << filePath;
        return false;
    }

    setBackgroundPixmap(pixmap);
    return true;
}

void CustomGraphicsScene::clearBackgroundPixmap()
{
    m_backgroundPixmap = QPixmap();
    m_boundaryConstraint = QRectF();
    setSceneRect(-5000, -5000, 10000, 10000);
    invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
}

bool CustomGraphicsScene::hasBackgroundPixmap() const
{
    return !m_backgroundPixmap.isNull();
}

const QPixmap& CustomGraphicsScene::backgroundPixmap() const
{
    return m_backgroundPixmap;
}

void CustomGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // 先填充背景色
    painter->fillRect(rect, m_backgroundColor);

    // 如果有背景图片，绘制背景图片
    if (hasBackgroundPixmap()) {
        painter->drawPixmap(0, 0, m_backgroundPixmap);
    }

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

void CustomGraphicsScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);

    // 绘制边界约束框（前景层，确保始终可见）
    if (hasBoundaryConstraint()) {
        QPen borderPen(QColor(100, 100, 100), 2);
        borderPen.setCosmetic(true);
        painter->setPen(borderPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(m_boundaryConstraint);
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
    // 获取当前选中的图元
    QList<QGraphicsItem*> selected = QGraphicsScene::selectedItems();
    if (selected.isEmpty()) {
        QGraphicsScene::mouseMoveEvent(event);
        return;
    }

    // 单选和多选使用不同的处理逻辑
    if (selected.size() == 1) {
        // 单选：使用原有逻辑
        handleSingleItemMove(event, selected);
    } else {
        // 多选：整体移动逻辑
        handleMultiItemMove(event, selected);
    }
}

void CustomGraphicsScene::handleSingleItemMove(QGraphicsSceneMouseEvent *event, const QList<QGraphicsItem*> &selected)
{
    QGraphicsItem *item = selected.first();

    // 记录移动前的位置
    QPointF oldPos = item->pos();

    // 调用父类处理默认移动
    QGraphicsScene::mouseMoveEvent(event);

    // 获取移动后的位置
    QPointF newPos = item->pos();

    // 如果没有实际移动，跳过
    QPointF delta = newPos - oldPos;
    if (qFuzzyIsNull(delta.x()) && qFuzzyIsNull(delta.y())) {
        return;
    }

    if (!CollisionHandler::isCollisionItem(item)) {
        // 仅应用边界约束
        applyBoundaryConstraint(item);
        return;
    }

    // 使用 shape() 的边界矩形
    QRectF itemShapeRect = item->shape().boundingRect();

    // 碰撞检测处理（如果启用）
    if (m_collisionEnabled) {
        // 获取源图元类型
        CollisionShapeType sourceType = CollisionHandler::getShapeType(item);

        // 获取场景中其他可能阻挡的图元
        QList<QGraphicsItem*> obstacles = getObstacles(item, sourceType);

        if (!obstacles.isEmpty()) {
            QRectF newItemRect = itemShapeRect.translated(newPos);

            for (QGraphicsItem *obstacle : obstacles) {
                QRectF obstacleRect = obstacle->shape().boundingRect().translated(obstacle->pos());

                if (newItemRect.intersects(obstacleRect)) {
                    QPointF blockedPos = calculateBlockedPosition(itemShapeRect, oldPos, newPos, obstacleRect);
                    item->setPos(blockedPos);
                    newPos = blockedPos;
                    newItemRect = itemShapeRect.translated(newPos);
                }
            }
        }

        // 边界约束
        applyBoundaryConstraint(item);
        newPos = item->pos();

        // 最终位置合法性验证：防止多阻挡物夹击或边界约束导致的瞬移重叠
        // 场景1：两阻挡物间距小于图元尺寸，处理后续阻挡物后回退到与前一个阻挡物重叠
        // 场景2：阻挡物紧靠边界，边界约束将图元推到与阻挡物重叠的位置
        QRectF finalItemRect = itemShapeRect.translated(newPos);
        for (QGraphicsItem *obstacle : obstacles) {
            QRectF obstacleRect = obstacle->shape().boundingRect().translated(obstacle->pos());
            if (finalItemRect.intersects(obstacleRect)) {
                // 最终位置与障碍物重叠，取消本次移动，保持原位置
                item->setPos(oldPos);
                return;
            }
        }
    } else {
        // 无碰撞检测时，仅应用边界约束
        applyBoundaryConstraint(item);
    }
}

void CustomGraphicsScene::handleMultiItemMove(QGraphicsSceneMouseEvent *event, const QList<QGraphicsItem*> &selected)
{
    // 记录所有图元移动前的位置
    QMap<QGraphicsItem*, QPointF> oldPositions;
    for (QGraphicsItem *item : selected) {
        oldPositions[item] = item->pos();
    }

    // 调用父类处理默认移动
    QGraphicsScene::mouseMoveEvent(event);

    // 计算第一个图元的移动偏移量（所有图元应该有相同的偏移）
    QGraphicsItem *firstItem = selected.first();
    QPointF firstOldPos = oldPositions[firstItem];
    QPointF firstNewPos = firstItem->pos();
    QPointF delta = firstNewPos - firstOldPos;

    // 如果没有实际移动，跳过
    if (qFuzzyIsNull(delta.x()) && qFuzzyIsNull(delta.y())) {
        return;
    }

    // 检测是否需要整体回退
    bool needRollback = false;
    QPointF minBlockedDelta = delta;  // 最小阻挡偏移量

    // 检查每个图元
    for (QGraphicsItem *item : selected) {
        if (!CollisionHandler::isCollisionItem(item)) {
            continue;
        }

        QPointF oldPos = oldPositions[item];
        QRectF itemShapeRect = item->shape().boundingRect();
        QRectF newItemRect = itemShapeRect.translated(oldPos + delta);

        // 碰撞检测
        if (m_collisionEnabled) {
            CollisionShapeType sourceType = CollisionHandler::getShapeType(item);
            QList<QGraphicsItem*> obstacles = getObstacles(item, sourceType);

            for (QGraphicsItem *obstacle : obstacles) {
                QRectF obstacleRect = obstacle->shape().boundingRect().translated(obstacle->pos());

                if (newItemRect.intersects(obstacleRect)) {
                    // 计算需要的阻挡偏移量
                    QPointF neededDelta = calculateMinBlockedDelta(itemShapeRect, oldPos, oldPos + delta, obstacleRect);
                    
                    // 更新最小阻挡偏移量（取所有阻挡中的最小移动）
                    if (qAbs(neededDelta.x()) < qAbs(minBlockedDelta.x()) ||
                        (minBlockedDelta.x() > 0 && neededDelta.x() < minBlockedDelta.x()) ||
                        (minBlockedDelta.x() < 0 && neededDelta.x() > minBlockedDelta.x())) {
                        minBlockedDelta.setX(neededDelta.x());
                    }
                    if (qAbs(neededDelta.y()) < qAbs(minBlockedDelta.y()) ||
                        (minBlockedDelta.y() > 0 && neededDelta.y() < minBlockedDelta.y()) ||
                        (minBlockedDelta.y() < 0 && neededDelta.y() > minBlockedDelta.y())) {
                        minBlockedDelta.setY(neededDelta.y());
                    }
                    needRollback = true;
                }
            }
        }

        // 边界约束检测
        if (hasBoundaryConstraint()) {
            QRectF testRect = itemShapeRect.translated(oldPos + delta);
            QPointF neededDelta = delta;
            bool hitBoundary = false;

            if (testRect.left() < m_boundaryConstraint.left()) {
                neededDelta.setX(delta.x() + (m_boundaryConstraint.left() - testRect.left()));
                hitBoundary = true;
            }
            if (testRect.right() > m_boundaryConstraint.right()) {
                neededDelta.setX(delta.x() - (testRect.right() - m_boundaryConstraint.right()));
                hitBoundary = true;
            }
            if (testRect.top() < m_boundaryConstraint.top()) {
                neededDelta.setY(delta.y() + (m_boundaryConstraint.top() - testRect.top()));
                hitBoundary = true;
            }
            if (testRect.bottom() > m_boundaryConstraint.bottom()) {
                neededDelta.setY(delta.y() - (testRect.bottom() - m_boundaryConstraint.bottom()));
                hitBoundary = true;
            }

            if (hitBoundary) {
                if (qAbs(neededDelta.x()) < qAbs(minBlockedDelta.x()) ||
                    (minBlockedDelta.x() > 0 && neededDelta.x() < minBlockedDelta.x()) ||
                    (minBlockedDelta.x() < 0 && neededDelta.x() > minBlockedDelta.x())) {
                    minBlockedDelta.setX(neededDelta.x());
                }
                if (qAbs(neededDelta.y()) < qAbs(minBlockedDelta.y()) ||
                    (minBlockedDelta.y() > 0 && neededDelta.y() < minBlockedDelta.y()) ||
                    (minBlockedDelta.y() < 0 && neededDelta.y() > minBlockedDelta.y())) {
                    minBlockedDelta.setY(neededDelta.y());
                }
                needRollback = true;
            }
        }
    }

    // 如果需要回退，将所有图元设置到最小阻挡偏移位置
    if (needRollback) {
        for (QGraphicsItem *item : selected) {
            QPointF oldPos = oldPositions[item];
            item->setPos(oldPos + minBlockedDelta);
        }
    }

    // 最终位置合法性验证：防止多阻挡物夹击或边界约束导致的瞬移重叠
    // 边界约束后的位置可能与障碍物重叠（当阻挡物紧靠边界且间距过窄）
    if (m_collisionEnabled) {
        bool hasOverlap = false;
        for (QGraphicsItem *item : selected) {
            if (!CollisionHandler::isCollisionItem(item)) {
                continue;
            }

            QRectF itemShapeRect = item->shape().boundingRect();
            QRectF finalItemRect = itemShapeRect.translated(item->pos());

            CollisionShapeType sourceType = CollisionHandler::getShapeType(item);
            QList<QGraphicsItem*> obstacles = getObstacles(item, sourceType);

            for (QGraphicsItem *obstacle : obstacles) {
                QRectF obstacleRect = obstacle->shape().boundingRect().translated(obstacle->pos());
                if (finalItemRect.intersects(obstacleRect)) {
                    hasOverlap = true;
                    break;
                }
            }
            if (hasOverlap) {
                break;
            }
        }

        // 如果最终位置与障碍物重叠，取消本次移动，恢复所有图元到原位置
        if (hasOverlap) {
            for (QGraphicsItem *item : selected) {
                item->setPos(oldPositions[item]);
            }
        }
    }
}

QList<QGraphicsItem*> CustomGraphicsScene::getObstacles(QGraphicsItem *source, CollisionShapeType sourceType)
{
    QList<QGraphicsItem*> allItems = QGraphicsScene::items();
    QList<QGraphicsItem*> obstacles;
    QList<QGraphicsItem*> selected = QGraphicsScene::selectedItems();

    for (QGraphicsItem *other : allItems) {
        if (other != source &&
            !selected.contains(other) &&
            CollisionHandler::isCollisionItemWithConfig(other, sourceType, m_collisionConfig)) {
            obstacles.append(other);
        }
    }
    return obstacles;
}

void CustomGraphicsScene::applyBoundaryConstraint(QGraphicsItem *item)
{
    if (!hasBoundaryConstraint()) {
        return;
    }

    QRectF itemSceneRect = item->shape().boundingRect().translated(item->pos());
    QPointF constrainedPos = item->pos();
    bool needsConstraint = false;

    if (itemSceneRect.left() < m_boundaryConstraint.left()) {
        constrainedPos.setX(item->pos().x() + (m_boundaryConstraint.left() - itemSceneRect.left()));
        needsConstraint = true;
    }
    if (itemSceneRect.right() > m_boundaryConstraint.right()) {
        constrainedPos.setX(item->pos().x() - (itemSceneRect.right() - m_boundaryConstraint.right()));
        needsConstraint = true;
    }
    if (itemSceneRect.top() < m_boundaryConstraint.top()) {
        constrainedPos.setY(item->pos().y() + (m_boundaryConstraint.top() - itemSceneRect.top()));
        needsConstraint = true;
    }
    if (itemSceneRect.bottom() > m_boundaryConstraint.bottom()) {
        constrainedPos.setY(item->pos().y() - (itemSceneRect.bottom() - m_boundaryConstraint.bottom()));
        needsConstraint = true;
    }

    if (needsConstraint) {
        item->setPos(constrainedPos);
    }
}

QPointF CustomGraphicsScene::calculateMinBlockedDelta(
    const QRectF &itemLocalRect, const QPointF &oldPos, const QPointF &newPos, const QRectF &obstacleRect)
{
    qreal dx = newPos.x() - oldPos.x();
    qreal dy = newPos.y() - oldPos.y();

    QRectF oldSceneRect = itemLocalRect.translated(oldPos);
    QPointF blockedDelta = QPointF(dx, dy);

    // 检查 Y 方向重叠
    bool yOverlap = (oldSceneRect.bottom() > obstacleRect.top() &&
                     oldSceneRect.top() < obstacleRect.bottom());
    // 检查 X 方向重叠
    bool xOverlap = (oldSceneRect.right() > obstacleRect.left() &&
                     oldSceneRect.left() < obstacleRect.right());

    // X 方向阻挡计算
    if (dx > 0 && yOverlap) {
        qreal maxRight = obstacleRect.left();
        qreal currentRight = oldSceneRect.right();
        if (currentRight < maxRight) {
            blockedDelta.setX(qMin(dx, maxRight - currentRight));
        } else {
            blockedDelta.setX(0);
        }
    } else if (dx < 0 && yOverlap) {
        qreal minLeft = obstacleRect.right();
        qreal currentLeft = oldSceneRect.left();
        if (currentLeft > minLeft) {
            blockedDelta.setX(qMax(dx, minLeft - currentLeft));
        } else {
            blockedDelta.setX(0);
        }
    }

    // Y 方向阻挡计算
    if (dy > 0 && xOverlap) {
        qreal maxBottom = obstacleRect.top();
        qreal currentBottom = oldSceneRect.bottom();
        if (currentBottom < maxBottom) {
            blockedDelta.setY(qMin(dy, maxBottom - currentBottom));
        } else {
            blockedDelta.setY(0);
        }
    } else if (dy < 0 && xOverlap) {
        qreal minTop = obstacleRect.bottom();
        qreal currentTop = oldSceneRect.top();
        if (currentTop > minTop) {
            blockedDelta.setY(qMax(dy, minTop - currentTop));
        } else {
            blockedDelta.setY(0);
        }
    }

    return blockedDelta;
}

QPointF CustomGraphicsScene::calculateBlockedPosition(
    const QRectF &itemLocalRect, const QPointF &oldPos, const QPointF &newPos, const QRectF &obstacleRect)
{
    // 计算移动方向和距离
    qreal dx = newPos.x() - oldPos.x();
    qreal dy = newPos.y() - oldPos.y();

    // 图元在 oldPos 的场景边界
    QRectF oldSceneRect = itemLocalRect.translated(oldPos);

    // 微小偏移量，避免浮点精度导致的粘连
    const qreal epsilon = 0;

    // 阻挡后的图元位置（初始为 oldPos）
    QPointF blockedPos = oldPos;

    // 检查 oldPos 是否已经与障碍物重叠
    if (oldSceneRect.intersects(obstacleRect)) {
        // 计算各个方向的推出距离
        qreal pushLeft = obstacleRect.left() - oldSceneRect.right() - epsilon;
        qreal pushRight = obstacleRect.right() - oldSceneRect.left() + epsilon;
        qreal pushUp = obstacleRect.top() - oldSceneRect.bottom() - epsilon;
        qreal pushDown = obstacleRect.bottom() - oldSceneRect.top() + epsilon;

        // 根据移动方向选择推出方向（选择推出距离较小的方向）
        bool shouldPushX = (dx > 0 && pushLeft < 0) || (dx < 0 && pushRight > 0);
        bool shouldPushY = (dy > 0 && pushUp < 0) || (dy < 0 && pushDown > 0);

        if (shouldPushX && shouldPushY) {
            // 两个方向都需要推出，选择推出距离较小的
            qreal pushDistX = (dx > 0) ? qAbs(pushLeft) : qAbs(pushRight);
            qreal pushDistY = (dy > 0) ? qAbs(pushUp) : qAbs(pushDown);
            if (pushDistX <= pushDistY) {
                blockedPos.setX(oldPos.x() + (dx > 0 ? pushLeft : pushRight));
                blockedPos.setY(newPos.y());
            } else {
                blockedPos.setY(oldPos.y() + (dy > 0 ? pushUp : pushDown));
                blockedPos.setX(newPos.x());
            }
        } else if (shouldPushX) {
            blockedPos.setX(oldPos.x() + (dx > 0 ? pushLeft : pushRight));
            blockedPos.setY(newPos.y());
        } else if (shouldPushY) {
            blockedPos.setY(oldPos.y() + (dy > 0 ? pushUp : pushDown));
            blockedPos.setX(newPos.x());
        } else {
            // 不需要推出，可以自由移动
            blockedPos = newPos;
        }

        return blockedPos;
    }

    // 检查图元与障碍物在 Y 方向上是否有重叠（用于判断 X 方向是否会被阻挡）
    bool yOverlap = (oldSceneRect.bottom() > obstacleRect.top() &&
                     oldSceneRect.top() < obstacleRect.bottom());
    // 检查图元与障碍物在 X 方向上是否有重叠（用于判断 Y 方向是否会被阻挡）
    bool xOverlap = (oldSceneRect.right() > obstacleRect.left() &&
                     oldSceneRect.left() < obstacleRect.right());

    // X 方向阻挡计算
    if (dx > 0 && yOverlap) {
        // 向右移动，且 Y 方向与障碍物重叠
        qreal maxRight = obstacleRect.left() - epsilon;
        qreal currentRight = oldSceneRect.right();
        if (currentRight < maxRight) {
            qreal maxDx = maxRight - currentRight;
            blockedPos.setX(oldPos.x() + qMin(dx, maxDx));
        } else {
            blockedPos.setX(oldPos.x());  // 已经在阻挡范围内，保持原位
        }
    } else if (dx < 0 && yOverlap) {
        // 向左移动，且 Y 方向与障碍物重叠
        qreal minLeft = obstacleRect.right() + epsilon;
        qreal currentLeft = oldSceneRect.left();
        if (currentLeft > minLeft) {
            qreal maxDx = minLeft - currentLeft;
            blockedPos.setX(oldPos.x() + qMax(dx, maxDx));
        } else {
            blockedPos.setX(oldPos.x());
        }
    } else {
        // X 方向不会被阻挡，可以自由移动
        blockedPos.setX(newPos.x());
    }

    // Y 方向阻挡计算
    if (dy > 0 && xOverlap) {
        // 向下移动，且 X 方向与障碍物重叠
        qreal maxBottom = obstacleRect.top() - epsilon;
        qreal currentBottom = oldSceneRect.bottom();
        if (currentBottom < maxBottom) {
            qreal maxDy = maxBottom - currentBottom;
            blockedPos.setY(oldPos.y() + qMin(dy, maxDy));
        } else {
            blockedPos.setY(oldPos.y());
        }
    } else if (dy < 0 && xOverlap) {
        // 向上移动，且 X 方向与障碍物重叠
        qreal minTop = obstacleRect.bottom() + epsilon;
        qreal currentTop = oldSceneRect.top();
        if (currentTop > minTop) {
            qreal maxDy = minTop - currentTop;
            blockedPos.setY(oldPos.y() + qMax(dy, maxDy));
        } else {
            blockedPos.setY(oldPos.y());
        }
    } else {
        // Y 方向不会被阻挡，可以自由移动
        blockedPos.setY(newPos.y());
    }

    return blockedPos;
}
