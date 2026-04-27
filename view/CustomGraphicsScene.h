#ifndef CUSTOMGRAPHICSSCENE_H
#define CUSTOMGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QColor>
#include <QHash>
#include <QPixmap>
#include "ShapeMetadata.h"
#include "../handlers/CollisionHandler.h"

class QGraphicsItem;

/**
 * @brief 自定义图形场景类，提供 2D 图形编辑器的公共场景基础设施。
 *
 * 继承自 QGraphicsScene，封装了网格背景绘制、场景默认配置等通用功能。
 * 支持双层网格（细网格 + 粗网格），可通过接口配置网格大小、颜色和开关。
 * 设计为可跨项目复用的基础组件。
 * 支持图元移动跟踪，用于撤销/重做功能。
 */
class CustomGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数，初始化场景并设置默认配置。
     *
     * 默认设置较大的场景区域（-5000, -5000, 10000, 10000），
     * 为编辑器提供充足的工作空间。
     *
     * @param parent 父 QObject 对象。
     */
    explicit CustomGraphicsScene(QObject *parent = nullptr);

    /**
     * @brief 设置是否绘制网格背景。
     *
     * @param enabled true 显示网格；false 隐藏网格。
     */
    void setGridEnabled(bool enabled);

    /**
     * @brief 获取网格背景是否启用。
     *
     * @return true 表示网格已启用；false 表示网格已隐藏。
     */
    bool isGridEnabled() const;

    /**
     * @brief 设置细网格的间距。
     *
     * @param size 细网格间距，单位：场景坐标像素，必须大于 0。
     */
    void setFineGridSize(qreal size);

    /**
     * @brief 获取细网格的间距。
     *
     * @return 细网格间距值。
     */
    qreal fineGridSize() const;

    /**
     * @brief 设置粗网格的间距。
     *
     * @param size 粗网格间距，单位：场景坐标像素，必须大于 0。
     */
    void setCoarseGridSize(qreal size);

    /**
     * @brief 获取粗网格的间距。
     *
     * @return 粗网格间距值。
     */
    qreal coarseGridSize() const;

    /**
     * @brief 设置细网格和粗网格的颜色。
     *
     * @param fine 细网格线条颜色。
     * @param coarse 粗网格线条颜色。
     */
    void setGridColors(const QColor &fine, const QColor &coarse);

    /**
     * @brief 设置场景背景颜色。
     *
     * @param color 背景颜色。
     */
    void setBackgroundColor(const QColor &color);

    /**
     * @brief 检查图元名称在场景中是否唯一。
     *
     * 基于 O(1) 哈希查找，判断指定名称是否已被其他图元占用。
     *
     * @param name 待检查的名称。
     * @param exclude 需要排除的图元（通常是正在重命名的图元自身），默认 nullptr。
     * @return true 表示名称唯一，可使用；false 表示名称已被其他图元占用。
     */
    bool isNameUnique(const QString &name, QGraphicsItem *exclude = nullptr) const;

    /**
     * @brief 根据名称查找图元。
     *
     * 基于 O(1) 哈希查找，返回场景中具有指定名称的图元。
     *
     * @param name 图元名称。
     * @return 对应的图元指针；未找到时返回 nullptr。
     */
    QGraphicsItem* findItemByName(const QString &name) const;

    /**
     * @brief 根据ID查找图元。
     *
     * 基于 O(1) 哈希查找，返回场景中具有指定 ID 的图元。
     *
     * @param id 图元ID。
     * @return 对应的图元指针；未找到时返回 nullptr。
     */
    QGraphicsItem* findItemById(int id) const;

    /**
     * @brief 更新图元名称索引。
     *
     * 当图元已存在于场景中且通过 setData 修改了名称后，
     * 必须调用此方法以同步维护内部名称哈希索引，否则后续查找会失效。
     *
     * @param item 被重命名的图元。
     * @param oldName 修改前的名称。
     * @param newName 修改后的名称。
     */
    void updateItemName(QGraphicsItem *item, const QString &oldName, const QString &newName);

    /**
     * @brief 清空名称和ID索引表。
     *
     * 在场景清空（clear）后调用，确保哈希索引与实际图元状态一致。
     * 正常情况下 addItem/removeItem 重写会自动维护索引，
     * 但 QGraphicsScene::clear() 不是虚函数，需手动补充清理。
     */
    void clearIndexes();

    // ========== 扩展基类方法（同名隐藏，维护索引） ==========

    /**
     * @brief 扩展 addItem，在添加图元到场景时同步维护名称和ID索引。
     *
     * 隐藏基类同名方法（addItem/removeItem 非虚函数），
     * 通过 CustomGraphicsScene* 指针调用时自动走此版本维护索引。
     *
     * @param item 要添加的图元。
     */
    void addItem(QGraphicsItem *item);

    /**
     * @brief 扩展 removeItem，在移除图元时同步清理名称和ID索引。
     *
     * 隐藏基类同名方法（addItem/removeItem 非虚函数），
     * 通过 CustomGraphicsScene* 指针调用时自动走此版本清理索引。
     *
     * @param item 要移除的图元。
     */
    void removeItem(QGraphicsItem *item);

    /**
     * @brief 设置碰撞配置。
     *
     * 用于配置哪些图元类型对之间可以发生碰撞。
     *
     * @param config 碰撞配置对象。
     */
    void setCollisionConfig(const CollisionConfig &config);

    /**
     * @brief 获取当前碰撞配置。
     *
     * @return 当前碰撞配置的引用。
     */
    const CollisionConfig& collisionConfig() const;

    /**
     * @brief 设置是否启用碰撞阻挡。
     *
     * @param enabled true 启用；false 禁用。
     */
    void setCollisionEnabled(bool enabled);

    /**
     * @brief 获取碰撞阻挡是否启用。
     *
     * @return true 已启用；false 已禁用。
     */
    bool isCollisionEnabled() const;

    // ========== 边界约束 ==========

    /**
     * @brief 设置图元移动边界约束。
     *
     * 限制图元移动的范围，超出边界的部分会被自动约束。
     * 主要用于石材切割场景，限制切割区域在石材范围内。
     *
     * @param boundary 边界矩形（场景坐标）；空矩形表示无约束。
     */
    void setBoundaryConstraint(const QRectF &boundary);

    /**
     * @brief 清除边界约束。
     */
    void clearBoundaryConstraint();

    /**
     * @brief 获取当前边界约束。
     *
     * @return 边界矩形；空矩形表示无约束。
     */
    QRectF boundaryConstraint() const;

    /**
     * @brief 检查是否设置了边界约束。
     *
     * @return true 已设置边界约束；false 无边界约束。
     */
    bool hasBoundaryConstraint() const;

    // ========== 背景图片 ==========

    /**
     * @brief 设置背景图片。
     *
     * 图片将作为场景背景绘制，并自动设置边界约束为图片大小。
     * 调用此方法后，场景将以图片尺寸作为绘制和移动的限制区域。
     *
     * @param pixmap 背景图片；空图片表示清除背景图片。
     */
    void setBackgroundPixmap(const QPixmap &pixmap);

    /**
     * @brief 从文件加载背景图片。
     *
     * @param filePath 图片文件路径。
     * @return true 加载成功；false 加载失败。
     */
    bool loadBackgroundFromFile(const QString &filePath);

    /**
     * @brief 清除背景图片。
     *
     * 同时清除边界约束。
     */
    void clearBackgroundPixmap();

    /**
     * @brief 检查是否设置了背景图片。
     *
     * @return true 已设置背景图片；false 无背景图片。
     */
    bool hasBackgroundPixmap() const;

    /**
     * @brief 获取背景图片。
     *
     * @return 背景图片的引用。
     */
    const QPixmap& backgroundPixmap() const;

signals:
    /**
     * @brief 图元移动完成信号。
     *
     * 当图元被拖拽移动后（鼠标释放时）发出，包含移动前后的位置信息。
     * 用于撤销栈记录等一次性操作。
     *
     * @param item 被移动的图元。
     * @param oldPos 移动前的位置。
     * @param newPos 移动后的位置。
     */
    void itemMoved(QGraphicsItem *item, const QPointF &oldPos, const QPointF &newPos);

    /**
     * @brief 图元正在移动信号。
     *
     * 在拖动过程中每次 mouseMoveEvent 时发出，用于属性面板等实时刷新场景。
     *
     * @param item 正在移动的图元。
     */
    void itemMoving(QGraphicsItem *item);

    /**
     * @brief 边界约束变化信号。
     *
     * 当场景边界约束发生变化时发出，包括设置、更新或清除约束。
     * 外部（如 DrawHandler）可连接此信号以同步自身的边界约束。
     *
     * @param boundary 新的边界约束矩形；空矩形表示无约束。
     */
    void boundaryConstraintChanged(const QRectF &boundary);

protected:
    /**
     * @brief 绘制场景背景，包含网格线。
     *
     * 重写 QGraphicsScene::drawBackground()，在背景上绘制双层网格。
     * 细网格使用浅色细线，粗网格使用略深的线条，提供视觉层次感。
     *
     * @param painter 用于绘制的 QPainter 对象。
     * @param rect 需要重绘的场景区域。
     */
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    /**
     * @brief 绘制场景前景，包含边界约束框。
     *
     * 重写 QGraphicsScene::drawForeground()，在前景层绘制边界约束框。
     * 前景层始终在所有图元之上，确保边界框不被其他图元覆盖。
     *
     * @param painter 用于绘制的 QPainter 对象。
     * @param rect 需要重绘的场景区域。
     */
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    /**
     * @brief 处理鼠标按下事件，记录选中图元的初始位置。
     *
     * @param event 鼠标事件。
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 处理鼠标移动事件，实现碰撞阻挡。
     *
     * @param event 鼠标事件。
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 处理鼠标释放事件，检测图元移动并发送信号。
     *
     * @param event 鼠标事件。
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 是否启用碰撞阻挡。
     *
     * true 启用碰撞阻挡；false 允许自由移动。
     */
    bool m_collisionEnabled;

    /**
     * @brief 计算图元被障碍物阻挡后的位置。
     *
     * @param itemRect 图元的边界矩形。
     * @param currentPos 图元当前位置。
     * @param targetPos 图元目标位置。
     * @param obstacleRect 障碍物边界。
     * @return 阻挡后的位置。
     */
    QPointF calculateBlockedPosition(const QRectF &itemRect, const QPointF &currentPos,
                                      const QPointF &targetPos, const QRectF &obstacleRect);

    /**
     * @brief 处理单个图元移动。
     *
     * @param event 鼠标事件。
     * @param selected 选中的图元列表。
     */
    void handleSingleItemMove(QGraphicsSceneMouseEvent *event, const QList<QGraphicsItem*> &selected);

    /**
     * @brief 处理多个图元整体移动。
     *
     * @param event 鼠标事件。
     * @param selected 选中的图元列表。
     */
    void handleMultiItemMove(QGraphicsSceneMouseEvent *event, const QList<QGraphicsItem*> &selected);

    /**
     * @brief 获取图元的障碍物列表。
     *
     * @param source 源图元。
     * @param sourceType 源图元碰撞类型。
     * @return 障碍物列表。
     */
    QList<QGraphicsItem*> getObstacles(QGraphicsItem *source, ShapeMeta::Type sourceType);

    /**
     * @brief 对图元应用边界约束。
     *
     * @param item 要约束的图元。
     */
    void applyBoundaryConstraint(QGraphicsItem *item);

    /**
     * @brief 计算最小阻挡偏移量。
     *
     * @param itemLocalRect 图元本地边界矩形。
     * @param oldPos 原始位置。
     * @param newPos 目标位置。
     * @param obstacleRect 障碍物边界。
     * @return 最小阻挡偏移量。
     */
    QPointF calculateMinBlockedDelta(const QRectF &itemLocalRect, const QPointF &oldPos,
                                      const QPointF &newPos, const QRectF &obstacleRect);

private:
    /**
     * @brief 是否绘制网格背景。
     *
     * true 表示绘制网格；false 表示仅绘制纯色背景。
     */
    bool m_gridEnabled;

    /**
     * @brief 细网格间距，单位：场景坐标像素。
     *
     * 默认值为 20.0。
     */
    qreal m_fineGridSize;

    /**
     * @brief 粗网格间距，单位：场景坐标像素。
     *
     * 默认值为 100.0，通常为细网格间距的整数倍。
     */
    qreal m_coarseGridSize;

    /**
     * @brief 细网格线条颜色。
     */
    QColor m_fineGridColor;

    /**
     * @brief 粗网格线条颜色。
     */
    QColor m_coarseGridColor;

    /**
     * @brief 场景背景颜色。
     */
    QColor m_backgroundColor;

    /**
     * @brief 记录图元移动前的位置。
     *
     * key 为图元指针，value 为位置。
     */
    QHash<QGraphicsItem*, QPointF> m_itemOldPositions;

    /**
     * @brief 名称到图元的索引表，用于 O(1) 名称查找和唯一性校验。
     */
    QHash<QString, QGraphicsItem*> m_nameIndex;

    /**
     * @brief ID到图元的索引表，用于 O(1) ID查找。
     */
    QHash<int, QGraphicsItem*> m_idIndex;

    /**
     * @brief 碰撞配置对象。
     *
     * 定义哪些图元类型对之间可以发生碰撞。
     */
    CollisionConfig m_collisionConfig;

    /**
     * @brief 边界约束矩形。
     *
     * 空矩形表示无约束。
     * 用于限制图元移动和绘制的范围（如石材切割场景）。
     */
    QRectF m_boundaryConstraint;

    /**
     * @brief 背景图片。
     *
     * 作为场景背景绘制，同时自动设置边界约束。
     */
    QPixmap m_backgroundPixmap;
};

#endif // CUSTOMGRAPHICSSCENE_H
