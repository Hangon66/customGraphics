#ifndef CUSTOMGRAPHICSSCENE_H
#define CUSTOMGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QColor>
#include <QHash>

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

signals:
    /**
     * @brief 图元移动完成信号。
     *
     * 当图元被拖拽移动后发出，包含移动前后的位置信息。
     *
     * @param item 被移动的图元。
     * @param oldPos 移动前的位置。
     * @param newPos 移动后的位置。
     */
    void itemMoved(QGraphicsItem *item, const QPointF &oldPos, const QPointF &newPos);

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
};

#endif // CUSTOMGRAPHICSSCENE_H
