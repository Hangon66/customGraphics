#ifndef COLLISIONHANDLER_H
#define COLLISIONHANDLER_H

#include "../view/ShapeMetadata.h"

#include <QRectF>
#include <QPointF>
#include <QList>
#include <QSet>
#include <QPair>

class QGraphicsItem;
class QGraphicsScene;

/**
 * @brief 碰撞配置类。
 *
 * 定义哪些图元类型对之间可以发生碰撞，以及碰撞边距。
 * 例如：仅矩形间碰撞、矩形与线条都碰撞等。
 * 支持像素（场景坐标）和毫米两种边距设置接口。
 */
class CollisionConfig
{
public:
    /**
     * @brief 构造函数，默认允许所有类型碰撞。
     */
    CollisionConfig();

    /**
     * @brief 析构函数。
     */
    ~CollisionConfig();

    /**
     * @brief 设置指定类型对是否允许碰撞。
     *
     * @param type1 第一个图元类型。
     * @param type2 第二个图元类型。
     * @param enabled true 允许碰撞；false 禁止碰撞。
     */
    void setCollisionEnabled(ShapeMeta::Type type1, ShapeMeta::Type type2, bool enabled);

    /**
     * @brief 检查指定类型对是否允许碰撞。
     *
     * @param type1 第一个图元类型。
     * @param type2 第二个图元类型。
     * @return true 允许碰撞；false 禁止碰撞。
     */
    bool isCollisionEnabled(ShapeMeta::Type type1, ShapeMeta::Type type2) const;

    /**
     * @brief 启用所有碰撞。
     */
    void enableAllCollisions();

    /**
     * @brief 禁用所有碰撞。
     */
    void disableAllCollisions();

    /**
     * @brief 仅启用矩形间碰撞。
     *
     * 线条不参与任何碰撞检测。
     */
    void enableRectOnlyCollision();

    /**
     * @brief 仅启用线条间碰撞。
     *
     * 矩形不参与任何碰撞检测。
     */
    void enableLineOnlyCollision();

    /**
     * @brief 启用矩形和线条间的所有碰撞。
     *
     * 包括：矩形-矩形、线条-线条、矩形-线条。
     */
    void enableRectAndLineCollision();

    // ========== 碰撞边距配置 ==========

    /**
     * @brief 设置碰撞边距（像素/场景坐标单位）。
     *
     * 碰撞检测时障碍物边界向四周扩展该值，使图元间保持指定间距。
     * 设置此值会覆盖之前通过 setCollisionMarginMM 设置的边距。
     *
     * @param margin 边距值，单位：场景坐标像素；0.0 表示无边距。
     */
    void setCollisionMargin(qreal margin);

    /**
     * @brief 获取碰撞边距（像素/场景坐标单位）。
     *
     * @return 当前边距值，单位：场景坐标像素。
     */
    qreal collisionMargin() const;

    /**
     * @brief 设置碰撞边距（毫米）。
     *
     * 内部通过 mmToSceneScale 将毫米转换为场景坐标单位存储。
     * 设置此值会覆盖之前通过 setCollisionMargin 设置的边距。
     *
     * @param mm 边距值，单位：毫米；0.0 表示无边距。
     */
    void setCollisionMarginMM(qreal mm);

    /**
     * @brief 获取碰撞边距（毫米）。
     *
     * 内部通过 mmToSceneScale 将存储的场景坐标值转换回毫米。
     *
     * @return 当前边距值，单位：毫米。
     */
    qreal collisionMarginMM() const;

    /**
     * @brief 设置毫米到场景坐标的缩放因子。
     *
     * 用于 setCollisionMarginMM / collisionMarginMM 的单位转换。
     * 例如：1场景坐标 = 1mm 时，scale = 1.0。
     * 默认值为 1.0，与项目 RulerHandler(1.0, "mm") 配置一致。
     *
     * @param scale 缩放因子，必须大于 0。
     */
    void setMmToSceneScale(qreal scale);

    /**
     * @brief 获取毫米到场景坐标的缩放因子。
     *
     * @return 当前缩放因子。
     */
    qreal mmToSceneScale() const;

private:
    /**
     * @brief 存储允许碰撞的类型对。
     *
     * 使用 QPair 存储类型对，(type1, type2) 和 (type2, type1) 视为相同。
     */
    QSet<QPair<int, int>> m_enabledPairs;

    /**
     * @brief 碰撞边距，单位：场景坐标像素。
     *
     * 碰撞检测时障碍物边界向四周扩展该值。默认 0.0 表示无边距。
     */
    qreal m_collisionMargin = 0.0;

    /**
     * @brief 毫米到场景坐标的缩放因子。
     *
     * 用于 mm 边距与场景坐标的相互转换：
     * margin_scene = margin_mm * m_mmToSceneScale。
     * 默认 1.0，即 1mm = 1场景坐标单位。
     */
    qreal m_mmToSceneScale = 1.0;
};

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
     * @param config 碰撞配置（可选）。
     * @return 如果点在任何图元内，返回该图元；否则返回 nullptr。
     */
    static QGraphicsItem* pointInAnyItem(QGraphicsScene *scene, const QPointF &point,
                                          QGraphicsItem *excludeItem = nullptr,
                                          const CollisionConfig *config = nullptr);

    /**
     * @brief 检查点是否在与源类型有碰撞配置的图元内部。
     *
     * 用于绘制模式下判断是否应该"绘制改选择"。
     *
     * @param scene 场景对象。
     * @param point 要检查的点（场景坐标）。
     * @param excludeItem 排除的图元。
     * @param sourceType 源图元类型（当前绘制类型）。
     * @param config 碰撞配置。
     * @return 如果点在与源类型有碰撞配置的图元内，返回该图元；否则返回 nullptr。
     */
    static QGraphicsItem* pointInAnyItemWithConfig(QGraphicsScene *scene, const QPointF &point,
                                                    QGraphicsItem *excludeItem,
                                                    ShapeMeta::Type sourceType,
                                                    const CollisionConfig &config);

    /**
     * @brief 检查矩形是否与任何可碰撞图元重叠。
     *
     * @param scene 场景对象。
     * @param rect 要检查的矩形（场景坐标）。
     * @param excludeItem 排除的图元。
     * @param config 碰撞配置（可选）。
     * @return 如果与任何图元重叠，返回该图元；否则返回 nullptr。
     */
    static QGraphicsItem* rectOverlapsAnyItem(QGraphicsScene *scene, const QRectF &rect,
                                               QGraphicsItem *excludeItem = nullptr,
                                               const CollisionConfig *config = nullptr);

    /**
     * @brief 计算图元在指定方向上可以移动的最大距离（受其他图元阻挡）。
     *
     * @param item 要移动的图元。
     * @param targetPos 目标位置。
     * @param otherItems 其他可能阻挡的图元列表。
     * @param margin 碰撞边距，障碍物边界向四周扩展该值；默认 0.0 无边距。
     * @return 考虑阻挡后的实际可移动位置。
     */
    static QPointF calculateBlockedPosition(QGraphicsItem *item, const QPointF &targetPos,
                                             const QList<QGraphicsItem*> &otherItems,
                                             qreal margin = 0.0);

    /**
     * @brief 计算两个矩形碰撞时的阻挡位置。
     *
     * @param movingRect 移动中的矩形。
     * @param targetRect 目标位置矩形。
     * @param obstacleRect 障碍物矩形。
     * @param margin 碰撞边距，障碍物边界向四周扩展该值；默认 0.0 无边距。
     * @return 阻挡后的位置（移动矩形左上角）。
     */
    static QPointF calculateRectBlockPosition(const QRectF &movingRect,
                                               const QRectF &targetRect,
                                               const QRectF &obstacleRect,
                                               qreal margin = 0.0);

    /**
     * @brief 判断图元是否参与碰撞检测（不考虑配置）。
     *
     * 图元需设置 ItemIsMovable 标志才参与碰撞。
     *
     * @param item 图元对象。
     * @return true 参与碰撞；false 不参与。
     */
    static bool isCollisionItem(QGraphicsItem *item);

    /**
     * @brief 判断图元是否参与碰撞检测（考虑配置）。
     *
     * @param item 图元对象。
     * @param sourceType 源图元类型（被拖动或正在绘制的图元类型）。
     * @param config 碰撞配置。
     * @return true 参与碰撞；false 不参与。
     */
    static bool isCollisionItemWithConfig(QGraphicsItem *item,
                                           ShapeMeta::Type sourceType,
                                           const CollisionConfig &config);

    /**
     * @brief 获取图元的碰撞类型。
     *
     * @param item 图元对象。
     * @return 图元的碰撞类型。
     */
    static ShapeMeta::Type getShapeType(QGraphicsItem *item);

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
