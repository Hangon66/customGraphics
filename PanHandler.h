#ifndef PANHANDLER_H
#define PANHANDLER_H

#include "IInteractionHandler.h"

#include <QPoint>

/**
 * @brief 鼠标拖拽平移交互处理器。
 *
 * 响应中键或右键拖拽操作，通过调整视图滚动条实现画布平移。
 * 拖拽过程中光标会变为抓手形状，提供直观的视觉反馈。
 * 触发平移的鼠标按键可通过接口配置。
 */
class PanHandler : public AbstractInteractionHandler
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     *
     * @param priority 处理器优先级，默认值 50。
     * @param parent 父 QObject 对象。
     */
    explicit PanHandler(int priority = 50, QObject *parent = nullptr);

    /**
     * @brief 处理鼠标按下事件，判断是否开始平移。
     *
     * 检查按下的鼠标按钮是否在配置的平移按钮集合中，
     * 如果匹配则进入平移状态，记录起始位置并改变光标。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标按下事件对象。
     * @return true 表示开始平移，事件被消费；false 表示不处理。
     */
    bool handleMousePress(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 处理鼠标移动事件，执行平移操作。
     *
     * 在平移状态下，计算鼠标位移量并调整视图滚动条位置。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标移动事件对象。
     * @return true 表示正在平移，事件被消费；false 表示不处理。
     */
    bool handleMouseMove(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 处理鼠标释放事件，结束平移操作。
     *
     * 恢复光标形状，清除平移状态。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标释放事件对象。
     * @return true 表示平移结束，事件被消费；false 表示不处理。
     */
    bool handleMouseRelease(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 获取处理器名称标识。
     *
     * @return 固定返回 "PanHandler"。
     */
    QString handlerName() const override;

    /**
     * @brief 设置触发平移的鼠标按键组合。
     *
     * @param buttons 鼠标按键标志位组合，如 Qt::MiddleButton | Qt::RightButton。
     */
    void setPanButtons(Qt::MouseButtons buttons);

    /**
     * @brief 获取当前触发平移的鼠标按键组合。
     *
     * @return 鼠标按键标志位组合。
     */
    Qt::MouseButtons panButtons() const;

private:
    /**
     * @brief 触发平移操作的鼠标按键组合。
     *
     * 默认值为 Qt::MiddleButton | Qt::RightButton。
     */
    Qt::MouseButtons m_panButtons;

    /**
     * @brief 当前是否处于平移拖拽状态。
     */
    bool m_isPanning;

    /**
     * @brief 上一帧鼠标在视口中的位置，用于计算位移量。
     */
    QPoint m_lastMousePos;
};

#endif // PANHANDLER_H
