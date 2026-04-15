#ifndef ZOOMHANDLER_H
#define ZOOMHANDLER_H

#include "IInteractionHandler.h"

/**
 * @brief 鼠标滚轮缩放交互处理器。
 *
 * 捕获滚轮事件，根据滚动方向对视图执行缩放操作。
 * 支持配置缩放因子、缩放范围限制以及是否以鼠标位置为缩放中心。
 * 提供重置缩放和自适应缩放的便利方法。
 */
class ZoomHandler : public AbstractInteractionHandler
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     *
     * @param priority 处理器优先级，默认值 100（滚轮缩放通常优先级最高）。
     * @param parent 父 QObject 对象。
     */
    explicit ZoomHandler(int priority = 100, QObject *parent = nullptr);

    /**
     * @brief 处理鼠标滚轮事件，执行视图缩放。
     *
     * 根据滚轮方向计算缩放因子，限制在 [m_minScale, m_maxScale] 范围内，
     * 如果启用了鼠标锚点，则以鼠标位置为缩放中心。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 滚轮事件对象。
     * @return true 表示事件已被消费。
     */
    bool handleWheel(QGraphicsView *view, QWheelEvent *event) override;

    /**
     * @brief 获取处理器名称标识。
     *
     * @return 固定返回 "ZoomHandler"。
     */
    QString handlerName() const override;

    /**
     * @brief 重置视图缩放为 1:1 原始比例。
     *
     * @param view 要重置缩放的 QGraphicsView 实例。
     */
    void resetZoom(QGraphicsView *view);

    /**
     * @brief 自适应缩放，使场景中所有图元完整显示在视图中。
     *
     * @param view 要执行自适应缩放的 QGraphicsView 实例。
     */
    void zoomToFit(QGraphicsView *view);

    /**
     * @brief 获取当前缩放级别。
     *
     * 基于最近一次 handleWheel 调用后的记录值，
     * 可能与视图实际 transform 不完全同步（如外部直接修改了 transform）。
     *
     * @return 当前缩放比例，1.0 表示原始大小。
     */
    qreal currentScale() const;

    /**
     * @brief 设置单次放大因子。
     *
     * @param factor 放大因子，必须大于 1.0。
     */
    void setZoomInFactor(qreal factor);

    /**
     * @brief 设置允许的最小缩放比例。
     *
     * @param scale 最小缩放值，必须大于 0。
     */
    void setMinScale(qreal scale);

    /**
     * @brief 设置允许的最大缩放比例。
     *
     * @param scale 最大缩放值，必须大于 m_minScale。
     */
    void setMaxScale(qreal scale);

    /**
     * @brief 设置是否以鼠标位置为缩放中心。
     *
     * @param enabled true 以鼠标位置为中心缩放；false 以视图中心缩放。
     */
    void setAnchorUnderMouse(bool enabled);

private:
    /**
     * @brief 单次放大因子。
     *
     * 默认值 1.15，缩小因子为其倒数。
     */
    qreal m_zoomInFactor;

    /**
     * @brief 允许的最小缩放比例。
     *
     * 默认值 0.05。
     */
    qreal m_minScale;

    /**
     * @brief 允许的最大缩放比例。
     *
     * 默认值 20.0。
     */
    qreal m_maxScale;

    /**
     * @brief 当前缩放级别的跟踪值。
     *
     * 默认值 1.0，在每次缩放操作后更新。
     */
    qreal m_currentScale;

    /**
     * @brief 是否以鼠标位置为缩放锚点。
     *
     * true 表示缩放时以鼠标所在位置为中心；
     * false 表示以视图中心为缩放中心。
     */
    bool m_anchorUnderMouse;
};

#endif // ZOOMHANDLER_H
