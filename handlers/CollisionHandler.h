#ifndef COLLISIONHANDLER_H
#define COLLISIONHANDLER_H

#include <QRectF>
#include <QPointF>
#include <QList>

class QGraphicsItem;
class QGraphicsScene;

/**
 * @brief 碰撞检测处理器。
 *
 * 提供图元间的碰撞检测和阻挡计算功能。
 * 支持刚性阻挡模式：拖动时被其他图元阻挡，绘制时检测点是否在图元内。
 */
class CollisionHandler
{
public:
    /**
     * @brief 构造函数。
     */
    CollisionHandler();

    /**
     * @brief 析构函数。
     */
    ~CollisionHandler();

    /**
     * @brief 检查点是否在任何可碰撞图元内部。
     *
     * @param scene 场景对象。
     * @param point 要检查的点（场景坐标）。
     * @param excludeItem 排除的图元（通常是被拖动的图元自身）。
     * @return 如果点在任何图元内，返回该图元；否则返回 nullptr。
     */
    static QGraphicsItem* pointInAnyItem(QGraphicsScene *scene, const QPointF &point,
                                          QGraphicsItem *excludeItem = nullptr);

    /**
     * @brief 检查矩形是否与任何可碰撞图元重叠。
     *
     * @param scene 场景对象。
     * @param rect 要检查的矩形（场景坐标）。
     * @param excludeItem 排除的图元。
     * @return 如果与任何图元重叠，返回该图元；否则返回 nullptr。
     */
    static QGraphicsItem* rectOverlapsAnyItem(QGraphicsScene *scene, const QRectF &rect,
                                               QGraphicsItem *excludeItem = nullptr);

    /**
     * @brief 计算图元在指定方向上可以移动的最大距离（受其他图元阻挡）。
     *
     * @param item 要移动的图元。
     * @param targetPos 目标位置。
     * @param otherItems 其他可能阻挡的图元列表。
     * @return 考虑阻挡后的实际可移动位置。
     */
    static QPointF calculateBlockedPosition(QGraphicsItem *item, const QPointF &targetPos,
                                             const QList<QGraphicsItem*> &otherItems);

    /**
     * @brief 计算两个矩形碰撞时的阻挡位置。
     *
     * @param movingRect 移动中的矩形。
     * @param targetRect 目标位置矩形。
     * @param obstacleRect 障碍物矩形。
     * @return 阻挡后的位置（移动矩形左上角）。
     */
    static QPointF calculateRectBlockPosition(const QRectF &movingRect,
                                               const QRectF &targetRect,
                                               const QRectF &obstacleRect);

    /**
     * @brief 判断图元是否参与碰撞检测。
     *
     * 图元需设置 ItemIsMovable 标志才参与碰撞。
     *
     * @param item 图元对象。
     * @return true 参与碰撞；false 不参与。
     */
    static bool isCollisionItem(QGraphicsItem *item);

private:
    /**
     * @brief 计算单个障碍物的阻挡位置。
     *
     * @param movingRect 移动矩形的当前边界。
     * @param obstacleRect 障碍物边界。
     * @param delta 移动增量。
     * @return 阻挡后的位置偏移。
     */
    static QPointF calculateSingleBlock(const QRectF &movingRect,
                                         const QRectF &obstacleRect,
                                         const QPointF &delta);
};

#endif // COLLISIONHANDLER_H
